#include "utils.h"

// defined in the header file
// template <typename T> 
// bool vector_contain(T item, vector<T> vec){
//     return find(vec.begin(), vec.end(), item) != vec.end();
// }
// template <typename T> 
// bool map_contain(string key, map<string, T> map){
//     return map.find(key) != map.end();
// }

bool is_str_integer(string str){
    return str.find_first_not_of("0123456789") == string::npos;
}
bool is_str_numeric(string str){
    int point_count = 0;
    for (size_t i = 0; i < str.length(); i++){
        if (str[i] == '.'){
            point_count++;
            if (point_count > 1) return false;
            continue;
        }
        if (str[i] < '0' || str[i] > '9') return false;
    }
    return true;
}
