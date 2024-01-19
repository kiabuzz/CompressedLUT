/*
----------------------------------------------------
 C        o       m      p     r    e   s  s ed  LUT 
----------------------------------------------------
*/
#include "compressedlut.h"

int main(int argc, char* argv[]) 
{
    string filename = "logo.txt";
    ifstream inputFile(filename);

    if (inputFile.is_open()) 
    {
        string line;
        while (std::getline(inputFile, line)) 
        {
            std::cout << line << std::endl;
        }
        inputFile.close();
    }

    bool is_table; 
    string table_path;
    string equation; int f_in = -1; int f_out = -1;
    string table_name = "compressedlut", output_path = ".";
    compressedlut::struct_configs configs = {2, 1, 1, 1};

    if ((argc < 2) || (argc % 2 == 0)) 
    {
        compressedlut::help();
        return 1;
    }
   
   for (int i = 1; i < argc; i = i + 2) 
   {
        string current_arg = argv[i];
        
        if (current_arg == "-table") {
            is_table = 1;
            table_path = argv[i + 1];
        } else if (current_arg == "-function") {
            is_table = 0;
            equation = argv[i + 1];
        } else if (current_arg == "-f_in") {
            f_in = stoi(argv[i + 1]);
        } else if (current_arg == "-f_out") {
            f_out = stoi(argv[i + 1]);
        } else if (current_arg == "-name") {
            table_name = argv[i + 1];
        } else if (current_arg == "-output") {
            output_path = argv[i + 1];
        } else if (current_arg == "-mdbw") {
            configs.mdbw = stoi(argv[i + 1]);
        } else if (current_arg == "-hbs") {
            configs.hbs = stoi(argv[i + 1]);
        } else if (current_arg == "-ssc") {
            configs.ssc = stoi(argv[i + 1]);
        } else if (current_arg == "-mlc") {
            configs.mlc = stoi(argv[i + 1]);
        } else
        {
            compressedlut::help();
            return 1;
        }
    }

    cout << "Results\n-------------------------------------------------------------------\n";
    vector<long int> table_data;
    bool is_signed = false;

    if(is_table)
    {
        ifstream table_file(table_path);
        if(table_file.is_open()) 
        {
            string line;
            while (getline(table_file, line)) {
                long int value;
                value = stoi(line, 0, 16);
                table_data.push_back(value);
            }
            table_file.close();

            if(table_data.size() != (1 << compressedlut::bit_width(table_data.size()-1)))
            {
                cerr << "Error: The table size must be of a power of 2." << endl;
                return 1;
            }
        } 
        else
        { 
            cerr << "Error: Could not open the table file." << endl;
            return 1;
        }
    }
    else
    {
        if(f_in < 0 || f_out < 0)
        {
            compressedlut::help();
            return 1;
        }

        double x, y;

        exprtk::symbol_table<double> symbol_table;
        symbol_table.add_variable("x", x);
        symbol_table.add_constants();
        exprtk::expression<double> expression;
        expression.register_symbol_table(symbol_table);
        exprtk::parser<double> parser;
        parser.compile(equation, expression);

        for(x = 0; x < 1; x = x + 1./(1 << f_in)) {
            y = expression.value();
            if(!isfinite(y))
            {
                cerr << "Error: The function is undefined at one or more points." << endl;
                return 1; 
            }
            table_data.push_back(round(y * (1 << f_out)));
        }

        long int min_value = *min_element(table_data.begin(), table_data.end());
        if(min_value < 0)
        {
            is_signed = true;
            long int max_value = *max_element(table_data.begin(), table_data.end());
            int w = compressedlut::bit_width_signed(min_value, max_value);
            for(int i =0; i < table_data.size(); i++)
            {
                if(table_data.at(i) < 0)
                    table_data.at(i) += ((long int)1 << w);
            }
        }
    }

    long int initial_size;
    vector<long int> final_size;

    compressedlut::compressedlut(table_data, table_name, output_path, configs, &initial_size, final_size);

    if(final_size.size() != 0)
    {
        cout << "Information: The table was compressed successfully!" << endl;

        int w_in = compressedlut::bit_width(table_data.size()-1);
        int w_out = compressedlut::bit_width(*max_element(table_data.begin(), table_data.end()));
        if(is_table)
        {
            cout << "\nInformation: The input and output bit width are as follows." << endl;
            cout << "--> Input Bit Width: " << w_in << endl;
            cout << "--> Output Bit Width: " << w_out  << endl;
        }
        else
        {
            cout << "\nInformation: The input and output values must be interpreted as follows." << endl;
            cout << "--> Input Format: Fixed Point <Unsigned, " << w_in << ", " << f_in << ">" << endl;
            if(is_signed)
                cout << "--> Output Format: Fixed Point <Signed, " << w_out << ", " << f_out << ">" << endl;
            else
                cout << "--> Output Format: Fixed Point <Unsigned, " << w_out << ", " << f_out << ">" << endl;
        }

        cout << "\nInformation: The results are as follows." << endl;
        for(int i = 0; i < final_size.size(); i++)
            cout << "-->  File Name: " << table_name << "_v" << i + 1 << "  Initial Size (bit): " << initial_size << "  Final Size (bit): " << final_size.at(i) << endl;
    }
    else
    {
        cout << "Information: Unable to compress the table!" << endl;
    }

    return 0;

}

void compressedlut::compressedlut(const vector<long int>& table_data, const string& table_name, const string& output_path, struct struct_configs configs, long int* initial_size, vector<long int>& final_size)
{
    vector<vector<long int>> all_cost;
    vector<int> all_w_in, all_w_out, all_w_l, all_w_s;
    vector<vector<long int>> all_t_lb, all_t_ust, all_t_bias, all_t_idx, all_t_rsh;
    
    vector<long int> t = table_data;

    while (true) 
    {
        int w_in = bit_width(t.size()-1);
        int w_out = bit_width(*max_element(t.begin(), t.end()));

        bool compressed = 0;
        long int best_cost = (1 << w_in) * w_out;
        int best_w_l, best_w_s;
        vector<long int> best_t_lb, best_t_ust, best_t_bias, best_t_idx, best_t_rsh;

        int max_w_l = (configs.hbs)? w_out-1 : 0;

        for (int w_l = 0; w_l <= max_w_l; w_l++) 
        {
            vector<long int> t_hb;
            vector<long int> t_lb;
            for(int i = 0; i < t.size(); i++)
            {
                t_hb.push_back(t.at(i) >> w_l);
                t_lb.push_back(t.at(i) & (((long int)1 << w_l) - 1));
            }
            
            long int cost_t_lb = (1 << w_in) * bit_width(*max_element(t_lb.begin(), t_lb.end()));

            for(int w_s = configs.mdbw; w_s < w_in ; w_s++)
            {
                vector<long int> t_ust, t_bias, t_idx, t_rsh;
                long int cost_t_hb = hb_compression(configs.ssc,t_hb, w_s, t_ust, t_bias, t_idx, t_rsh);
                if((cost_t_hb + cost_t_lb) < best_cost)
                {
                    compressed = 1;
                    best_cost = cost_t_hb + cost_t_lb;
                    best_w_l = w_l;
                    best_w_s = w_s;
                    best_t_lb = t_lb;
                    best_t_ust = t_ust;
                    best_t_bias = t_bias;
                    best_t_idx = t_idx;
                    best_t_rsh = t_rsh;
                }
            }
        }

        if(compressed)
        {
            vector<long int> cost = {(1 << w_in) * w_out, best_cost};
            all_cost.push_back(cost);
            all_w_in.push_back(w_in);
            all_w_out.push_back(w_out);
            all_w_l.push_back(best_w_l);
            all_w_s.push_back(best_w_s);
            all_t_lb.push_back(best_t_lb);
            all_t_ust.push_back(best_t_ust);
            all_t_bias.push_back(best_t_bias);
            all_t_idx.push_back(best_t_idx);
            all_t_rsh.push_back(best_t_rsh);

            if((best_t_bias.size() >= (1 << (configs.mdbw+1))) && configs.mlc)
                t = best_t_bias; 
            else
                break;
        }
        else
            break;
    }

    if(all_w_in.size() != 0)
    {
        *initial_size = all_cost.at(0).at(0);
        final_size.push_back(all_cost.at(0).at(1));
        for(int i = 1; i < all_w_in.size(); i++)
            final_size.push_back(final_size.at(i-1) - all_cost.at(i).at(0) + all_cost.at(i).at(1));

        for(int i = 0; i < all_w_in.size(); i++)
        {
            string rtl_file_path = output_path + "/" + table_name + "_v" + to_string(i + 1) + ".v";
            string hls_file_path = output_path + "/" + table_name + "_v" + to_string(i + 1) + ".cpp";
            rtl(rtl_file_path, table_name, all_w_in, all_w_out, all_w_l, all_w_s, all_t_lb, all_t_ust, all_t_bias, all_t_idx, all_t_rsh, i + 1);
            hls(hls_file_path, table_name, all_w_in, all_w_out, all_w_l, all_w_s, all_t_lb, all_t_ust, all_t_bias, all_t_idx, all_t_rsh, i + 1);
        }
    }
}

long int compressedlut::hb_compression(bool ssc, const vector<long int>& t_hb, int w_s, vector<long int>& t_ust, vector<long int>& t_bias, vector<long int>& t_idx, vector<long int>& t_rsh)
{
    const int w_in = bit_width(t_hb.size()-1);
    const int w_out = bit_width(*max_element(t_hb.begin(), t_hb.end()));
    const long int num_sub_table = (1 << (w_in - w_s));
    const  int len_sub_table = (1 << w_s);

    vector<long int> t_st(num_sub_table*len_sub_table);

    for(int i = 0; i < num_sub_table; i++) 
    {
        long int bias = *min_element(t_hb.begin() + i*len_sub_table, t_hb.begin() + (i+1)*len_sub_table);
        t_bias.push_back(bias);
        for(int j = 0; j < len_sub_table; j++)
            t_st.at(i*len_sub_table+j) = t_hb.at(i*len_sub_table+j) - bias;
    }

    if(!ssc)
    {
        t_ust = t_st;
        int w_bias = bit_width(*max_element(t_bias.begin(), t_bias.end()));
        int w_ust =  bit_width(*max_element(t_ust.begin(), t_ust.end()));
        return (1 << w_in) * w_ust + (1 << (w_in - w_s)) * w_bias; 
    }
    else
    {
        std::vector<std::vector<bool>> sm(num_sub_table, std::vector<bool>(num_sub_table, false));
        std::vector<std::vector<int>> sm_rsh(num_sub_table, std::vector<int>(num_sub_table, 0));
        vector<long int> sv(num_sub_table);
        for(long int i = 0; i < num_sub_table; i++)
        {
            for(long int j = i+1; j < num_sub_table; j++)
            {
                for(int rsh = 0; rsh < 4; rsh++)
                {
                    bool i_generates_j = true, j_generates_i = true;
                    for (int p = 0; p < len_sub_table; p++)
                    {
                        long int value_i = t_st.at(i*len_sub_table+p);
                        long int value_j = t_st.at(j*len_sub_table+p);
                        i_generates_j = i_generates_j && ((value_i >> rsh) == value_j);
                        j_generates_i = j_generates_i && ((value_j >> rsh) == value_i);
                        if(i_generates_j == false && j_generates_i == false)
                            break;
                    }
                    
                    if(i_generates_j)
                    {
                        sm.at(i).at(j) = 1;
                        sv.at(i)++;
                        sm_rsh.at(i).at(j) = rsh;
                        
                    }
                    if(j_generates_i)
                    {
                        sm.at(j).at(i) = 1;
                        sv.at(j)++;
                        sm_rsh.at(j).at(i) = rsh;
                    }
                    if(i_generates_j || j_generates_i)
                        break;
                }

            }
        }

        t_idx.resize(num_sub_table);
        t_rsh.resize(num_sub_table);
        vector<bool> sub_table_derived(num_sub_table, false);
        long int unique_idx = 0;

        while(any_of(sv.begin(), sv.end(), [](bool v){return v;}))
        {
            long int idx_unique = distance(sv.begin(), max_element(sv.begin(), sv.end()));
            t_idx.at(idx_unique) = unique_idx;
            t_rsh.at(idx_unique) = 0;
            sub_table_derived.at(idx_unique) = true;
            for(long int idx = 0; idx < num_sub_table; idx++)
            {
                if(sm.at(idx_unique).at(idx))
                {
                    t_idx.at(idx) = unique_idx;
                    t_rsh.at(idx) = sm_rsh.at(idx_unique).at(idx);
                    sub_table_derived.at(idx) = true;
                    fill(sm.at(idx).begin(), sm.at(idx).end(), false);
                    sv.at(idx) = 0;
                    for(long int i = 0; i < num_sub_table; i++)
                    {
                        if(sm.at(i).at(idx))
                        {
                            sm.at(i).at(idx) = false;
                            sv.at(i)--;
                        }
                    }
                }
            }

            for(long int i = 0; i < num_sub_table; i++)
            {
                if(sm.at(i).at(idx_unique))
                {
                    sm.at(i).at(idx_unique) = false;
                    sv.at(i)--;
                }
            }
            sv.at(idx_unique) = 0;

            for(int i = 0; i < len_sub_table; i++)
                t_ust.push_back(t_st.at(idx_unique*len_sub_table+i));

            unique_idx++;
        }

        
        for(long int i = 0; i < num_sub_table; i++)
        {
            if(!sub_table_derived.at(i))
            {
                t_idx.at(i) = unique_idx;
                t_rsh.at(i) = 0;

                for(int j = 0; j < len_sub_table; j++)
                    t_ust.push_back(t_st.at(i*len_sub_table+j));

                unique_idx++;
            }
        }

    }

    int w_ust = bit_width(*max_element(t_ust.begin(), t_ust.end()));
    int w_bias = bit_width(*max_element(t_bias.begin(), t_bias.end()));
    int w_rsh = bit_width(*max_element(t_rsh.begin(), t_rsh.end()));
    int w_idx = bit_width(*max_element(t_idx.begin(), t_idx.end()));
    return t_ust.size() * w_ust + (1 << (w_in - w_s)) * (w_bias + w_rsh + w_idx);
}

void compressedlut::rtl(const string& file_path, const string& table_name, const vector<int>& all_w_in, const vector<int>& all_w_out, const vector<int>& all_w_l, const vector<int>& all_w_s, const vector<vector<long int>>& all_t_lb, const vector<vector<long int>>& all_t_ust, const vector<vector<long int>>& all_t_bias, const vector<vector<long int>>& all_t_idx, const vector<vector<long int>>& all_t_rsh, int max_level)
{
    ofstream file_init(file_path);
    file_init.close();

    for (int level = max_level; level >= 1; level--) 
    {
        const vector<long int> t_lb = all_t_lb.at(level-1);
        const vector<long int> t_ust = all_t_ust.at(level-1);
        const vector<long int> t_bias = all_t_bias.at(level-1);
        const vector<long int> t_idx = all_t_idx.at(level-1);
        const vector<long int> t_rsh = all_t_rsh.at(level-1);

        const int w_in = all_w_in.at(level-1);
        const int w_out = all_w_out.at(level-1);
        const int w_l = all_w_l.at(level-1);
        const int w_s = all_w_s.at(level-1);

        const int w_lb = (t_lb.size() == 0)? 0 : bit_width(*max_element(t_lb.begin(), t_lb.end()));
        const int w_ust = (t_ust.size() == 0)? 0 : bit_width(*max_element(t_ust.begin(), t_ust.end()));
        const int w_bias = (t_bias.size() == 0)? 0 : bit_width(*max_element(t_bias.begin(), t_bias.end()));
        const int w_idx = (t_idx.size() == 0)? 0 : bit_width(*max_element(t_idx.begin(), t_idx.end()));
        const int w_rsh = (t_rsh.size() == 0)? 0 : bit_width(*max_element(t_rsh.begin(), t_rsh.end()));

        if(w_ust != 0) 
        {
            string name = table_name + "_ust_" + to_string(level);
            plaintable_rtl(file_path, name, t_ust);
        }
        if(level == max_level && w_bias != 0)
        {
            string name = table_name + "_bias_" + to_string(level);
            plaintable_rtl(file_path, name, t_bias);
        }
        if (w_idx != 0) 
        {
            string name = table_name + "_idx_" + to_string(level);
            plaintable_rtl(file_path, name, t_idx);
        }
        if (w_rsh != 0) 
        {
            string name = table_name + "_rsh_" + to_string(level);
            plaintable_rtl(file_path, name, t_rsh);
        }
        if (w_lb != 0) 
        {
            string name = table_name + "_lb_" + to_string(level);
            plaintable_rtl(file_path, name, t_lb);
        }

        ofstream file(file_path, ios::app);

        if(level == 1)
            file << "\nmodule " << table_name << "(address, data);\n";
        else 
            file << "\nmodule " << table_name << "_" << level << "(address, data);\n";
        file << "input wire [" << w_in - 1 << ":0] address;\n";
        file << "output reg [" << w_out - 1 << ":0] data;\n\n";


        if(w_idx != 0)
            file << "wire [" << w_idx - 1 << ":0] i; " << table_name << "_idx_" << level << " idx_" << level << "_inst(address[" << w_in - 1 << ":" << w_s << "], i);\n";
      
        if (w_rsh != 0)
            file << "wire [" << w_rsh - 1 << ":0] t; " << table_name << "_rsh_" << level << " rsh_" << level << "_inst(address[" << w_in - 1 << ":" << w_s << "], t);\n";

        if (w_bias != 0) 
        {
            if (level == max_level) 
                file << "wire [" << w_bias - 1 << ":0] b; " << table_name << "_bias_" << level << " bias_" << level << "_inst(address[" << w_in - 1 << ":" << w_s << "], b);\n";
            else
                file << "wire [" << w_bias - 1 << ":0] b; " << table_name << "_" << level + 1 << " " << table_name << "_" << level + 1  << "_inst(address[" << w_in - 1 << ":" << w_s << "], b);\n";
        }

        if (w_lb != 0)
            file << "wire [" << w_lb - 1 << ":0] lb; " << table_name << "_lb_" << level << " lb_" << level << "_inst(address, lb);\n";

        if (w_ust != 0) 
        {
            if (w_idx != 0) 
                file << "wire [" << w_ust - 1 << ":0] u; " << table_name << "_ust_" << level << " ust_" << level << "_inst({i, address[" << w_s - 1 << ":0]}, u);";
            else 
            {
                if(t_idx.size() == 0)
                    file << "wire [" << w_ust - 1 << ":0] u; " << table_name << "_ust_" << level << " ust_" << level << "_inst(address, u);";
                else
                    file << "wire [" << w_ust - 1 << ":0] u; " << table_name << "_ust_" << level << " ust_" << level << "_inst(address[" << w_s - 1 << ":0], u);";
            }
        }

        file << "\n\nalways @(*) begin\n";

        if (w_l != 0)
            file << "\tdata = {";
        else 
            file << "\tdata = ";
        
        if (w_ust != 0 && w_rsh != 0 && w_bias != 0)
            file << "(u >> t) + b";
        else if (w_ust != 0 && w_rsh != 0 && w_bias == 0) 
            file << "(u >> t)";
        else if (w_ust != 0 && w_rsh == 0 && w_bias != 0)
            file << "u + b";
        else if (w_ust != 0 && w_rsh == 0 && w_bias == 0)
            file << "u";
        else if (w_ust == 0 && w_bias != 0)
            file << "b";
        else 
            file << "'" << (w_out - w_l) << "'d0";

        if (w_l != 0) 
            if(w_l == w_lb)
                file << ", lb};\n";
            else if(w_lb != 0)
                file << ", " << (w_l - w_lb) << "'d0, lb};\n";
            else
                file << ", " << w_l << "'d0};\n";
        else
            file << ";\n";

        file << "end\nendmodule\n";

        file.close();
    }

}

void compressedlut::plaintable_rtl(const string& file_path, const string& table_name, const vector<long int>& table_data) 
{
    ofstream file(file_path, ios::app);

    int w_in = bit_width(table_data.size() - 1);
    int w_out = bit_width(*max_element(table_data.begin(), table_data.end()));

    file << "\nmodule " << table_name << "(address, data);\n";
    file << "input wire [" << w_in - 1 << ":0] address;\n";
    file << "output reg [" << w_out - 1 << ":0] data;\n\n";
    file << "always @(*) begin\n";
    file << "\tcase(address)\n";
    for (long int i = 0; i < table_data.size(); i++) 
    {
        file << "\t\t" << w_in << "'d" << i << ": ";
        file << "data = " << w_out << "'d" << table_data.at(i) << ";\n";
    }
    file << "\t\tdefault: data = " << w_out << "'d0" << ";\n\tendcase\nend\n";
    file << "endmodule\n";

    file.close();
}


void compressedlut::hls(const string& file_path, const string& table_name, const vector<int>& all_w_in, const vector<int>& all_w_out, const vector<int>& all_w_l, const vector<int>& all_w_s, const vector<vector<long int>>& all_t_lb, const vector<vector<long int>>& all_t_ust, const vector<vector<long int>>& all_t_bias, const vector<vector<long int>>& all_t_idx, const vector<vector<long int>>& all_t_rsh, int max_level)
{
    ofstream file(file_path);
    file << "#include <ap_int.h>\n";
    file.close();

    for (int level = max_level; level >= 1; level--) 
    {
        const vector<long int> t_lb = all_t_lb.at(level-1);
        const vector<long int> t_ust = all_t_ust.at(level-1);
        const vector<long int> t_bias = all_t_bias.at(level-1);
        const vector<long int> t_idx = all_t_idx.at(level-1);
        const vector<long int> t_rsh = all_t_rsh.at(level-1);

        const int w_in = all_w_in.at(level-1);
        const int w_out = all_w_out.at(level-1);
        const int w_l = all_w_l.at(level-1);
        const int w_s = all_w_s.at(level-1);

        const int w_lb = (t_lb.size() == 0)? 0 : bit_width(*max_element(t_lb.begin(), t_lb.end()));
        const int w_ust = (t_ust.size() == 0)? 0 : bit_width(*max_element(t_ust.begin(), t_ust.end()));
        const int w_bias = (t_bias.size() == 0)? 0 : bit_width(*max_element(t_bias.begin(), t_bias.end()));
        const int w_idx = (t_idx.size() == 0)? 0 : bit_width(*max_element(t_idx.begin(), t_idx.end()));
        const int w_rsh = (t_rsh.size() == 0)? 0 : bit_width(*max_element(t_rsh.begin(), t_rsh.end()));
        
        file.open(file_path, ios::app);

        if(level == 1)
            file << "\nvoid " << table_name << "(ap_uint<" << w_in << "> address, ap_uint<" << w_out << ">* data) {\n";
        else 
            file << "\nvoid " << table_name << "_" << level << "(ap_uint<" << w_in << "> address, ap_uint<" << w_out << ">* data) {\n";

        file << "\t#pragma HLS PIPELINE\n";

        file.close();

        if(w_ust != 0) 
        {
            string name = table_name + "_ust_" + to_string(level);
            plaintable_hls(file_path, name, t_ust);
        }
        if(level == max_level && w_bias != 0)
        {
            string name = table_name + "_bias_" + to_string(level);
            plaintable_hls(file_path, name, t_bias);
        }
        if (w_idx != 0) 
        {
            string name = table_name + "_idx_" + to_string(level);
            plaintable_hls(file_path, name, t_idx);
        }
        if (w_rsh != 0) 
        {
            string name = table_name + "_rsh_" + to_string(level);
            plaintable_hls(file_path, name, t_rsh);
        }
        if (w_lb != 0) 
        {
            string name = table_name + "_lb_" + to_string(level);
            plaintable_hls(file_path, name, t_lb);
        }

        file.open(file_path, ios::app);

        file << "\n";

        if(w_idx != 0)
            file << "\tap_uint<" << w_idx << "> i = " << table_name << "_idx_" << level  << "[address.range(" << w_in - 1 << ", " << w_s << ")];\n";
      
        if (w_rsh != 0)
            file << "\tap_uint<" << w_rsh << "> t = " << table_name << "_rsh_" << level  << "[address.range(" << w_in - 1 << ", " << w_s << ")];\n";

        if (w_bias != 0) 
        {
            if (level == max_level) 
            {
                file << "\tap_uint<" << w_bias << "> b = " << table_name << "_bias_" << level  << "[address.range(" << w_in - 1 << ", " << w_s << ")];\n";
            }
            else
            {
                file << "\tap_uint<" << w_bias << "> b; " << table_name << "_" << level+1  << "(address.range(" << w_in - 1 << ", " << w_s << "), &b);\n";
            }
        }

        if (w_lb != 0)
            file << "\tap_uint<" << w_lb << "> lb = " << table_name << "_lb_" << level  << "[address];\n";

        if (w_ust != 0) 
        {
            if (w_idx != 0) 
            {
                file << "\tap_uint<" << w_idx+w_s << "> ust_idx; ust_idx.range(" << w_idx+w_s-1 << ", " << w_s << ") = i; ust_idx.range(" << w_s-1 << ", 0) = address.range(" << w_s-1 <<", 0); ";
                file << "ap_uint<" << w_ust << "> u = " << table_name << "_ust_" << level  << "[ust_idx];\n";
            }
            else 
            {
                if(t_idx.size() == 0)
                    file << "\tap_uint<" << w_ust << "> u = " << table_name << "_ust_" << level  << "[address];\n";
                else
                    file << "\tap_uint<" << w_ust << "> u = " << table_name << "_ust_" << level  << "[" << "address.range(" << w_s-1 <<", 0)" <<"];\n";
            }
        }

        if (w_l != 0)
            file << "\tap_uint<" << w_out << "> data_t; data_t.range(" << w_out-1 <<", " << w_l <<") = ";
        else 
            file << "\tap_uint<" << w_out << "> data_t = ";
        
        if (w_ust != 0 && w_rsh != 0 && w_bias != 0)
            file << "(u >> t) + b;\n";
        else if (w_ust != 0 && w_rsh != 0 && w_bias == 0) 
            file << "(u >> t);\n";
        else if (w_ust != 0 && w_rsh == 0 && w_bias != 0)
            file << "u + b;\n";
        else if (w_ust != 0 && w_rsh == 0 && w_bias == 0)
            file << "u;\n";
        else if (w_ust == 0 && w_bias != 0)
            file << "b;\n";
        else 
            file << "0;\n";

        if (w_l != 0) 
        {   if(w_lb != 0)
                file << "\tdata_t.range(" << w_l-1 <<", 0) = lb;\n";
            else
                file << "\tdata_t.range(" << w_l-1 <<", 0) = 0;\n";
        }

        file << "\n\t*data = data_t;\n}\n";

        file.close();
    }

}

void compressedlut::plaintable_hls(const string& file_path, const string& table_name, const vector<long int>& table_data) 
{
    ofstream file(file_path, ios::app);

    int w_in = bit_width(table_data.size() - 1);
    int w_out = bit_width(*max_element(table_data.begin(), table_data.end()));

    file << "\n\tconst ap_uint<" << w_out << "> " << table_name << "[" << table_data.size() << "] = {";
    for (long int i = 0; i < table_data.size(); i++) 
    {
        file << table_data.at(i);
        if(i == (table_data.size()-1))
            file << "};\n";
        else
            file << ", ";
    }

    file << "\t#pragma HLS bind_storage variable=" << table_name << " type=ROM_1P impl=lutram\n";

    file.close();
}

int compressedlut::bit_width(long int value) 
{
    if (value == 0)
        return 0;
    return floor(log2(abs(value)))+1;
}

int compressedlut::bit_width_signed(long int min_value, long int max_value) 
{
    int w = 1;
    while(1)
    {
        if(min_value >= -((long int)1 << (w-1)) && max_value <= (((long int)1 << (w-1))-1))
            return w;
        else
            w++;
    }
}

void compressedlut::help() 
{
    string filename = "help.txt";
    ifstream inputFile(filename);
    if (inputFile.is_open()) 
    {
        string line;
        while (std::getline(inputFile, line)) 
        {
            std::cout << line << std::endl;
        }
        inputFile.close();
    }
}
