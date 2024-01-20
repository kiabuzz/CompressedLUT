/*
----------------------------------------------------
 C        o       m      p     r    e   s  s ed  LUT 
----------------------------------------------------
*/
#  ifndef __COMPRESSEDLUT_H
#  define __COMPRESSEDLUT_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <numeric>
#include "exprtk.hpp"

using namespace std;

namespace compressedlut {
    struct struct_configs {int mdbw;  bool hbs; bool ssc; bool mlc;};

    void compressedlut(const vector<long int>& table_data, const string& table_name, const string& output_path, struct struct_configs configs, long int* initial_size, vector<long int>& final_size);
    long int hb_compression(bool self_similarity, const vector<long int>& t_hb, int w_s, vector<long int>& t_ust, vector<long int>& t_bias, vector<long int>& t_idx, vector<long int>& t_rsh);
    void rtl(const string& file_path, const string& table_name, const vector<int>& all_w_in, const vector<int>& all_w_out, const vector<int>& all_w_l, const vector<int>& all_w_s, const vector<vector<long int>>& all_t_lb, const vector<vector<long int>>& all_t_ust, const vector<vector<long int>>& all_t_bias, const vector<vector<long int>>& all_t_idx, const vector<vector<long int>>& all_t_rsh, int max_level);
    void plaintable_rtl(const string& file_path, const string& table_name, const vector<long int>& table_data);
    void hls(const string& file_path, const string& table_name, const vector<int>& all_w_in, const vector<int>& all_w_out, const vector<int>& all_w_l, const vector<int>& all_w_s, const vector<vector<long int>>& all_t_lb, const vector<vector<long int>>& all_t_ust, const vector<vector<long int>>& all_t_bias, const vector<vector<long int>>& all_t_idx, const vector<vector<long int>>& all_t_rsh, int max_level);
    void plaintable_hls(const string& file_path, const string& table_name, const vector<long int>& table_data);
    int bit_width(long int value);
    int bit_width_signed(long int min_value, long int max_value);
    void help();
}

# endif
