# CompressedLUT
Lookup tables are widely used in hardware applications to store arrays of constant values. They can be directly used to evaluate nonlinear functions or used as a part of other approximate methods (e.g., piecewise linear approximation and bipartite tables) to compute such functions. CompressedLUT is a tool for lossless compression of lookup tables and generation of their hardware files in Verilog and C++ for RTL and HLS designs. 

CompressedLUT has been developed as a part of the following publication. Please refer to it for more information.
> Alireza Khataei and Kia Bazargan. 2024. CompressedLUT: An Open Source Tool for Lossless Compression of Lookup Tables for Function Evaluation and Beyond. In Proceedings of the 2024 ACM/SIGDA International Symposium on Field Programmable Gate Arrays (FPGA ’24), March 3–5, 2024, Monterey, CA, USA. ACM, New York, NY, USA, 10 pages. https://doi.org/10.1145/3626202.3637575

## Authors
- Alireza Khataei, University of Minnesota, Minneapolis, MN, USA
- Kia Bazargan (kia@umn.edu), University of Minnesota, Minneapolis, MN, USA


## Installation
```bash
git clone https://github.com/kiabuzz/CompressedLUT.git
cd CompressedLUT
make
```
    
## Getting Started
This tool works in two modes: you can either (1) specify  a  text  file  that contains the values of a lookup table, or (2) describe a math function by providing its equation.

#### 1. Lookup Table as a Text File
In this mode, you need to prepare a text (.txt) file, containing the values of your lookup table. The file must contain a power of 2 lines, each of which is a single hexadecimal value. An example of such a text file can be found in example.txt. The following command generates hardware files corresponding to the lookup table described in that text file.

```bash
./compressedlut -table example.txt
```

#### 2. Lookup Table as a Math Equation
In this mode, you need to specify a math function by providing its equation and fixed-point quantization parameters. The following command is an example of this mode.

```bash
./compressedlut -function "exp(x-1)" -f_in 10 -f_out 12
```

The command above generates hardware files for implementing the math function exp(x-1). The function is evaluated in [0, 1) using 10 fractional bits for the input and 12 fractional bits for the output. The signedness and bit width of the integer part in the output are determined automatically. To evaluate this function in ranges other than [0, 1), you can modify the equation by scaling and shifting the input variable x.

It is worth noting that you can use your customized script to evaluate a math function, then store its output values in a text file, and finally compress and implement it using CompressedLUT in the first mode.

See the help.txt file for command line arguments in more detail.

## Citation
CompressedLUT has been developed as a part of the following publication.

```bibtex
@inproceedings{compressedlut_fpga,
    author = {Khataei, Alireza and Bazargan, Kia},
    title = {CompressedLUT: An Open Source Tool for Lossless Compression of Lookup Tables for Function Evaluation and Beyond},
    year = {2024},
    publisher = {Association for Computing Machinery},
    address = {New York, NY, USA},
    url = {https://doi.org/10.1145/3626202.3637575},
    doi = {10.1145/3626202.3637575},
    booktitle = {Proceedings of the 2024 ACM/SIGDA International Symposium on Field Programmable Gate Arrays},
    pages = {2-11},
    numpages = {10},
    location = {Monterey, CA, USA},
    series = {FPGA '24}
}
```

> Alireza Khataei and Kia Bazargan. 2024. CompressedLUT: An Open Source Tool for Lossless Compression of Lookup Tables for Function Evaluation and Beyond. In Proceedings of the 2024 ACM/SIGDA International Symposium on Field Programmable Gate Arrays (FPGA ’24), March 3–5, 2024, Monterey, CA, USA. ACM, New York, NY, USA, 10 pages. https://doi.org/10.1145/3626202.3637575

## Copyright & License Notice
The CompressedLUT package is copyrighted by the Regents of the University of Minnesota. It can be freely used for educational and research purposes by non-profit institutions and US government agencies only. Other organizations are allowed to use CompressedLUT only for evaluation purposes, and any further uses will require prior approval. The software may not be sold or redistributed without prior approval. One may make copies of the software for their use provided that the copies, are not sold or distributed, are used under the same terms and conditions.

As unestablished research software, this code is provided on an ``as is'' basis without warranty of any kind, either expressed or implied. The downloading, or executing any part of this software constitutes an implicit agreement to these terms. These terms and conditions are subject to change at any time without prior notice.


## Acknowledgements
This material is based upon work supported in part by Cisco Systems, Inc. under grant number 00105407, and by the National Science Foundation under grant number PFI-TT 2016390.
