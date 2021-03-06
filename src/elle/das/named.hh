#pragma once

#include <utility>

#include <elle/Option.hh>
#include <elle/Printable.hh>
#include <elle/attribute.hh>
#include <elle/log.hh>
#include <elle/meta.hh>
#include <elle/serialization/Serializer.hh>

#include <elle/das/Symbol.hh>

namespace elle
{
  namespace das
  {
    namespace named
    {
      template <typename ... Args>
      using List = elle::meta::List<Args...>;

      /*--------.
      | Helpers |
      `--------*/

      template <typename T>
      T
      _make_formal(...);

      template <typename T>
      typename T::Formal
      _make_formal(int);

      template <typename T>
      using make_formal =
        std::remove_reference_t<decltype(_make_formal<T>(42))>;

      template <typename T>
      T
      _make_symbol(...);

      template <typename T>
      typename T::Symbol
      _make_symbol(int);

      template <typename T>
      using make_symbol =
        std::remove_reference_t<decltype(_make_symbol<T>(42))>;

      template <typename Formal, typename Effective>
      static
      constexpr std::enable_if_exists_t<typename Effective::Formal, int>
      lookup(int)
      {
        return std::is_same<make_formal<Formal>,
                            typename Effective::Formal>::value ? 1 : 2;
      };

      template <typename Formal, typename Effective>
      static
      constexpr int
      lookup(...)
      {
        return 0;
      };

      template <typename T>
      struct is_effective
      {
        static constexpr bool value =
          !std::is_same<decltype(_make_formal<T>(42)), T>::value;
      };

      template <bool effective, typename T>
      struct DefaultFor;

      /*-----.
      | Find |
      `-----*/

      template <int Index,
                int Current,
                typename Formal,
                typename LEffective,
                typename REffective>
      struct find;

      template <int lookup,
                int Index,
                int Current,
                typename Formal,
                typename LEffective,
                typename REffective>
      struct find_impl;

      // Recurse
      template <int Lookup,
                int Index,
                int Current,
                typename Formal,
                typename Effective,
                typename ... LEffectives,
                typename ... REffectives>
      struct find_impl<Lookup,
                       Index,
                       Current,
                       Formal,
                       elle::meta::List<LEffectives...>,
                       elle::meta::List<Effective, REffectives...>>
        : public find<Index,
                      Current + 1,
                      Formal,
                      elle::meta::List<LEffectives..., Effective>,
                      elle::meta::List<REffectives...>>
      {};

      // Positional
      template <int Current,
                typename Formal,
                typename Effective,
                typename ... LEffectives,
                typename ... REffectives>
      struct find_impl<0,
                       Current,
                       Current,
                       Formal,
                       elle::meta::List<LEffectives...>,
                       elle::meta::List<Effective, REffectives...>>
      {
        using type = Effective;
        template <typename Default>
        static
        auto
        get(Default&, LEffectives&& ..., Effective&& e, REffectives&& ...)
          -> decltype(std::forward<Effective>(e))
        {
          return std::forward<Effective>(e);
        }
      };

      // Named.
      template <int Index,
                int Current,
                typename Formal,
                typename Effective,
                typename ... LEffectives,
                typename ... REffectives>
      struct find_impl<1,
                       Index,
                       Current,
                       Formal,
                       elle::meta::List<LEffectives...>,
                       elle::meta::List<Effective, REffectives...>>
      {
        template <typename Default>
        static
        typename Effective::Passing
        get(Default&, LEffectives&& ..., Effective&& e, REffectives&& ...)
        {
          return std::forward<typename Effective::Passing>(e.value);
        }
      };

      // Switch.
      template <int Index,
                int Current,
                typename Formal,
                typename Effective,
                typename ... LEffectives,
                typename ... REffectives>
      struct find<Index,
                  Current,
                  Formal,
                  elle::meta::List<LEffectives...>,
                  elle::meta::List<Effective, REffectives...>>
        : public find_impl<lookup<Formal, Effective>(0),
                           Index,
                           Current,
                           Formal,
                           elle::meta::List<LEffectives...>,
                           elle::meta::List<Effective, REffectives...>>
      {};

      // Not found, use default.
      template <int Index,
                int Current,
                typename Formal,
                typename ... Effectives>
      struct find<Index,
                  Current,
                  Formal,
                  elle::meta::List<Effectives...>,
                  elle::meta::List<>>
      {
        template <typename Default>
        static
        auto&
        get(Default& d, Effectives&& ...)
        {
          return d.DefaultFor<true, Formal>::value;
        }
      };

      /*-------------.
      | DefaultStore |
      `-------------*/

      template <bool Effective, typename T>
      struct DefaultFor
      {
        static constexpr bool has = false;
        template <typename Formal>
        DefaultFor(Formal const&)
        {}
      };

      template <typename T>
      struct DefaultFor<true, T>
        : public T::Formal::
            template Effective<typename T::Type, typename T::Type const&>
      {
        static constexpr bool has = true;
        using type = typename T::Type;
        using Super = typename T::Formal::
          template Effective<typename T::Type, typename T::Type const&>;
        using Super::Super;
      };

      template <typename ... Formal>
      struct DefaultStore
        : public DefaultFor<is_effective<Formal>::value, Formal>...
      {
        template <typename ... Args>
        DefaultStore(Args&& ... args)
          : DefaultFor<is_effective<Formal>::value, Formal>(
            std::forward<Args>(args))...
        {
          static_assert(sizeof ... (Formal) == sizeof ... (Args), "LOLEUH2");
        }

        template <typename T>
        using default_for = DefaultFor<is_effective<T>::value, T>;

        template <typename ... Args>
        auto
        extend(Args&& ... args) const
        {
          return DefaultStore<Args ..., Formal ...>(
            std::forward<Args>(args)...,
            static_cast<DefaultFor<is_effective<Formal>::value, Formal> const&>(
              *this)...);
        }
      };

      /*----------.
      | Prototype |
      `----------*/

      template <typename ... Formal>
      struct Prototype
        : public elle::Printable::as<Prototype<Formal...>>
      {
        ELLE_LOG_COMPONENT("elle.das.named");
        using DefaultStore = named::DefaultStore<Formal...>;
        DefaultStore defaults;
        Prototype(DefaultStore&& d)
          : defaults(std::move(d))
        {}

        template <typename... NewArgs>
        auto
        extend(NewArgs&& ... args) const
        {
          return Prototype<NewArgs..., Formal...>
            (this->defaults.extend(std::forward<NewArgs>(args)...));
        }

        template <int Index, typename F, typename ... Args>
        using find = named::find<Index,
                                 0,
                                 std::remove_cv_reference_t<F>,
                                 elle::meta::List<>,
                                 elle::meta::List<Args...>>;

        template <typename Sequence>
        struct Call;

        template <typename I, I ... Index>
        struct Call<std::integer_sequence<I, Index ...>>
        {
          template <typename Default, typename F, typename ... Args>
          static
          auto
          call(Default& defaults, F const& f, Args&& ... args)
          {
            return f(find<Index, Formal, Args...>::get(
                       defaults, std::forward<Args>(args)...)...);
          }

          template <typename Default, typename ... Args>
          static
          auto
          map(Default& defaults, Args&& ... args)
          {
            return std::make_tuple(find<Index, Formal, Args...>::get(
                                     defaults, std::forward<Args>(args)...)...);
          }
        };

        template <typename F, typename ... Args>
        auto
        call(F const& f, Args&& ... args) const
        {
          return Call<std::make_index_sequence<sizeof ... (Formal)>>::call(
            this->defaults, f, std::forward<Args>(args)...);
        }

        template <typename ... Args>
        auto
        map(Args&& ... args) const
        {
          return Call<std::make_index_sequence<sizeof ... (Formal)>>::map(
            this->defaults, std::forward<Args>(args)...);
        }

        template <typename Arg>
        struct print_prototype
        {
          using type = bool;
          static
          bool
          value(std::ostream& o, bool& first)
          {
            if (first)
              first = false;
            else
              o << ", ";
            o << Arg::name();
            return true;
          }
        };

        void
        print(std::ostream& o) const
        {
          bool first = true;
          elle::meta::List<std::remove_cv_reference_t<Formal>...>::
            template map<print_prototype>::value(o, first);
        }
      };

      template <typename ... Args>
      Prototype<Args...>
      prototype(Args&& ... args)
      {
        return Prototype<Args...>
          (DefaultStore<Args...>(std::forward<Args>(args)...));
      }

      /*---------.
      | Function |
      `---------*/

      template <typename Signature>
      class Function;

      namespace
      {
        template <typename T>
        struct get_type_impl
        {
          using type = typename T::Type;
        };

        template <typename T>
        using get_type = typename get_type_impl<T>::type;
      }

      template <typename R, typename ... Args>
      class Function<R (Args...)>
        : public Printable::as<Function<R (Args...)>>
      {
      public:
        using F = std::function<R (get_type<Args> ...)>;
        template <typename ... CArgs>
        Function(F f, CArgs&& ... args)
          : _function(std::move(f))
          , _prototype(DefaultStore<make_symbol<Args>...>(
                         std::forward<CArgs>(args)...))
        {
          static_assert(sizeof ... (Args) == sizeof ... (args), "LOLEUH");
        }

        template <typename ... Effective>
        auto
        operator() (Effective&& ... effective) const
        {
          return this->_prototype.call(
            this->_function, std::forward<Effective>(effective)...);
        }

        template <typename T>
        using make_effective = typename make_symbol<T>::template Effective<
          std::remove_cv_reference_t<get_type<T>>,
          std::remove_cv_reference_t<get_type<T>>>;

        class Call
          : public make_effective<Args>...
        {
        public:
          Call(get_type<Args> const& ... args)
            : make_effective<Args>(args)...
          {}

          Call(elle::serialization::SerializerIn& s)
            : make_effective<Args>(
                s.deserialize<std::remove_cv_reference_t<get_type<Args>>>(
                  make_symbol<Args>::name()))...
          {}

          void
          serialize(serialization::Serializer& s)
          {
            auto ignore = {
              (s.serialize(make_symbol<Args>::name(),
                           this->make_effective<Args>::value), 0)...
            };
            (void)ignore;
          }
        };

        class Result
          : public elle::Option<R, std::exception_ptr>
        {
        public:
          using elle::Option<R, std::exception_ptr>::Option;
          R
          operator()()
          {
            if (this->template is<R>())
              return this->template get<R>();
            else
              std::rethrow_exception(this->template get<std::exception_ptr>());
          }
        };

        template <typename ... Effective>
        Call
        call(Effective&& ... args)
        {
          return this->_prototype.call(
            [] (get_type<Args> const& ... args)
            {
              return Call(args...);
            }, std::forward<Effective>(args)...);
        }

        template <typename C>
        std::enable_if_t<
          std::is_same<std::remove_cv_reference_t<C>, Call>::value, Result>
        operator() (C&& c) const
        {
          try
          {
            return Result(this->_function(std::forward<C>(c).make_effective<Args>::value...));
          }
          catch (elle::Exception const& e)
          {
            return Result(std::current_exception());
          }
        }

        ELLE_ATTRIBUTE_R(F, function);
        ELLE_ATTRIBUTE_R(
          (Prototype<make_symbol<Args> ...>), prototype);

        void
        print(std::ostream& out) const
        {
          elle::fprintf(
            out, "%s[%s]", this->_function, this->_prototype);
        }
      };

      namespace
      {
        template <typename T, typename Named>
        typename Named::template Formal<T>
        make_signature_arg(...);

        template <typename T, typename Named>
        typename Named::template rebind<T, T>
        make_signature_arg(int);

        template <typename S, typename ... Named>
        struct make_signature;

        template <typename R, typename ... Args, typename ... Named>
        struct make_signature<R (Args...), Named...>
        {
          using type = R (decltype(make_signature_arg<Args, Named>(0)) ...);
        };
      }

      template <typename F, typename ... Args>
      Function<
        typename make_signature<std::get_signature<F>,
                                std::remove_cv_reference_t<Args>...>::type>
      function(F f, Args&& ... args)
      {
        return Function<
          typename make_signature<
            std::get_signature<F>,
            std::remove_cv_reference_t<Args>...>::type>(
              std::move(f), std::forward<Args>(args)...);
      }
    }
  }
}
