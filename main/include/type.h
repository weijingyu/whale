#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <filesystem>

template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T, typename ... Args>
constexpr Ref<T> CreateRef(Args&& ... args) {
	return std::make_shared<T>(std::forward<Args>(args)...);
}

typedef std::string ID;
using String = std::string;

template<typename K, typename V>
using Map = std::unordered_map<K, V>;

template<typename T>
using Vec = std::vector<T>;

template<typename T>
using Set = std::unordered_set<T>;

