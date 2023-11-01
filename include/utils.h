#pragma once

#include <vector>
#include <string>
#include <map>

using std::vector; 
using std::string;
using std::map;

template <typename T> 
bool vector_contain(T item, vector<T> vec){
    return find(vec.begin(), vec.end(), item) != vec.end();
}
template <typename T> 
bool map_contain(string key, map<string, T> map){
    return map.find(key) != map.end();
}

bool is_str_integer(string str);
bool is_str_numeric(string str);
