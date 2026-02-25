#pragma once
#include <map>
#include <filesystem>
#include <type_traits>
#include "magic_enum.hpp"

class DebugParameter
{
public:
	DebugParameter() = delete;
	~DebugParameter() = delete;

	enum Dimension { NIL, PLAIN_TYPE, READ_ONLY, HIDDEN };

	static void reset(const std::filesystem::path& name) {
		const std::filesystem::path parent = "..";
		for (auto it = params.lower_bound(name); it != params.upper_bound(name);) {
			auto rel = it->first.lexically_relative(name);
			if (rel.empty() || *rel.begin() != parent) {
				it = params.erase(it);
			}
			else {
				break;
			}
		}
	}

	template<class T, Dimension t = Dimension::PLAIN_TYPE>
	static T get(const std::filesystem::path& name, const T& def) {
		if constexpr (std::is_enum_v<T>) {
			return selected<T>(name, def);
		}
		T* ref = getRef<T, t>(name, def);
		if (!ref) return def;
		return *ref;
	}

	template<class T, Dimension t = Dimension::PLAIN_TYPE>
	static void set(const std::filesystem::path& name, const T& val) {
		if constexpr (std::is_enum_v<T>) {
			auto e = getRef<enum_t>(name, edef);
			if (!e) return;
			e->val = static_cast<uint32_t>(val);
			return;
		}
		T* ref = getRef<T, t>(name, val);
		if (!ref) return;
		*ref = val;
	}

	static bool popTrigger(const std::filesystem::path& name) {
		trigger_t def{};
		auto trig = get<trigger_t>(name, def);
		if (trig.head > trig.tail) {
			trig.tail = trig.head;
			return true;
		}
		return false;		
	}

	static void pushTrigger(const std::filesystem::path& name) {
		trigger_t def{};
		auto trig = get<trigger_t>(name, def);
		++trig.head;
	}

	
private:
	template<class T, Dimension t>
	static T* getRef(const std::filesystem::path& name, const T& def) {
		static_assert(t != Dimension::NIL);
		static_assert(std::is_copy_assignable_v<T> && std::is_copy_constructible_v<T>);
		auto& v = params[name];
		if (v.dim == Dimension::NIL) {
			v.dim = t;
			v.typ = reinterpret_cast<size_t>(typei<T>);
			if constexpr (sizeof(T) > 16) {
				v.dtor = destructor<T>;
				*reinterpret_cast<T**>(v.s) = new T(def);
			}
			else {
				if constexpr (!std::is_trivially_destructible_v<T>) { v.dtor = destructor<T>; }
				new (v.s) T(def);
			}
		}
		else if (v.typ != reinterpret_cast<size_t>(typei<T>)) {
			return nullptr;
		}
		if constexpr (sizeof(T) > 16) {
			return **reinterpret_cast<T**>(v.s);
		}
		else {
			return *reinterpret_cast<T*>(v.s);
		}
	}

	template<class E>
	E selected(const std::filesystem::path& name, E def) {
		static_assert(std::is_enum_v<E> && sizeof(E) <= 4);
		enum_t edef{ static_cast<uint32_t>(def), rt_enumerate<E> };
		auto e = getRef<enum_t>(name, edef);
		if (!e) return def;
		return (E)e->val;
	}

	template<class T> static size_t typei() { T* t{}; return reinterpret_cast<size_t>(&t); }
	template<class T>
	static void destructor(void* p) {
		if constexpr(sizeof(T) > 16) {
			delete reinterpret_cast<T*>(p);
		}
		else if constexpr (!std::is_trivially_destructible_v<T>) {
			reinterpret_cast<T*>(p)->~T();
		}
		magic_enum::enum_entries<T> e;
	}

	struct trigger_t {
		uint32_t head = 0;
		uint32_t tail = 0;
	};

	template<class T>
	static void rt_enumerate(std::function<void(uint32_t, const std::string&)> f) {
		static_assert
		for(auto& kv : magic_enum::enum_entries<T>) {
			f(kv.first, kv.second);
		}
	}

	struct enum_t {
		uint32_t val = 0;
		decltype(rt_enumerate<Dimension>)* en = nullptr;
	};

	struct variant16 {
	public:
		friend class DebugParameter;
		uint8_t s[16];
		~variant16() {
			if(dtor) dtor(s);
		}
	private:
		Dimension dim = Dimension::NIL;
		size_t typ = 0;
		decltype(destructor<int>)* dtor;
	};
	inline static std::map<std::filesystem::path, variant16> params;
};