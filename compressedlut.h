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

    void compressedlut(const vector<int>& table_data, const string& table_name, const string& output_path, struct struct_configs configs, int* initial_size, vector<int>& final_size);
    int hb_compression(bool self_similarity, const vector<int>& t_hb, int w_s, vector<int>& t_ust, vector<int>& t_bias, vector<int>& t_idx, vector<int>& t_rsh);
    void rtl(const string& file_path, const string& table_name, const vector<int>& all_w_in, const vector<int>& all_w_out, const vector<int>& all_w_l, const vector<int>& all_w_s, const vector<vector<int>>& all_t_lb, const vector<vector<int>>& all_t_ust, const vector<vector<int>>& all_t_bias, const vector<vector<int>>& all_t_idx, const vector<vector<int>>& all_t_rsh, int max_level);
    void plaintable_rtl(const string& file_path, const string& table_name, const vector<int>& table_data);
    void hls(const string& file_path, const string& table_name, const vector<int>& all_w_in, const vector<int>& all_w_out, const vector<int>& all_w_l, const vector<int>& all_w_s, const vector<vector<int>>& all_t_lb, const vector<vector<int>>& all_t_ust, const vector<vector<int>>& all_t_bias, const vector<vector<int>>& all_t_idx, const vector<vector<int>>& all_t_rsh, int max_level);
    void plaintable_hls(const string& file_path, const string& table_name, const vector<int>& table_data);
    int bit_width(int value);
    void help();
}

# endif
