# CompressedLUT
A tool to generate optimized hardware files for univariate functions.

Written by: 
	     Alireza Khataei, Kia Bazargan, University of Minnesota
	     kia@umn.edu


Copyright & License Notice
-------------------------------------------------------------------
The CompressedLUT package is copyrighted  by  the  Regents  of  the
University of Minnesota. It can be freely used for educational  and
research purposes  by  non-profit  institutions  and  US government
agencies only. Other organizations are allowed to use CompressedLUT
only for evaluation purposes, and any  further  uses  will  require 
prior approval. The software  may  not  be  sold  or  redistributed 
without prior approval. One may make copies  of  the  software  for 
their use provided that the copies, are  not  sold  or distributed, 
are used under the same terms and conditions.

As unestablished research software, this code  is  provided  on  an
``as is'' basis without warranty of any kind, either  expressed  or
implied. The downloading, or executing any part  of  this  software
constitutes an implicit agreement to these terms. These  terms  and
conditions are subject to change at any time without  prior notice.
-------------------------------------------------------------------
Instructions
-------------------------------------------------------------------

This program generates source code  for  univariate  functions.  It 
works in two modes: you can either (1) specify  a  .txt  file  that 
contains the values of a look-up table specifying f(x)  for  all  x 
values, or (2) describe the function by providing its equation.

1) Lookup table as .txt  File  -->  Please  specify  the  following 
   arguments:
	-table (string)		: the path of the table,  in  which 
	                          each line has a  single   integer  
	                          value.  The  table  size must  be 
	                          a power of 2. 
	
2) Function equation --> Please specify the following arguments:
	-function (string)	: the  function's  equation  as   a 
	                          string in quotation marks "".
	-f_in (int)		: fractional  bit  width   of   the 
                                  function's   input.   Since   the 
                                  function is evaluated in  [0, 1),
                                  there are  no  integer  bits  for 
                                  the input.
	-f_out (int)		: fractional  bit  width   of   the 
	                          function's  output.  The   output 
	                          integer bit width  is  determined 
	                          automatically.


The following optional arguments can be used in either mode.
	-name (string)		: output  table   and   file   name
	-output (string)	: the   output   folder   for   the 
	                          generated files. The folder  must
	                          already exist.
	-mdbw (int)		: minimum decomposition bit  width. 
	                          The default value is 2. 
	-hbs (0/1)		: higher-bit  split.  The   default 
	                          value is 1 (0=no, 1=yes). 
	-ssc (0/1)		: self-similarity compression.  The 
	                          default value is 1. 
	-mlc (0/1)		: multi-level   compression.    The 
	                          default value is 1.
