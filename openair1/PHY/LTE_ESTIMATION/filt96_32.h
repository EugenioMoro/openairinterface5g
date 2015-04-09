/*******************************************************************************
    OpenAirInterface
    Copyright(c) 1999 - 2014 Eurecom

    OpenAirInterface is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.


    OpenAirInterface is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenAirInterface.The full GNU General Public License is
   included in this distribution in the file called "COPYING". If not,
   see <http://www.gnu.org/licenses/>.

  Contact Information
  OpenAirInterface Admin: openair_admin@eurecom.fr
  OpenAirInterface Tech : openair_tech@eurecom.fr
  OpenAirInterface Dev  : openair4g-devel@eurecom.fr

  Address      : Eurecom, Campus SophiaTech, 450 Route des Chappes, CS 50193 - 06904 Biot Sophia Antipolis cedex, FRANCE

 *******************************************************************************/
short filt24_0[24] = {
  2730,5461,8192,10922,13653,16384,13653,10922,8192,5461,2730,0,0,0,0,0,0,0,0,0,0,0,0,0
};
short filt24_0_dcl[24] = {
  2341,4681,7022,9362,11703,16384,13653,10922,8192,5461,2730,0,0,0,0,0,0,0,0,0,0,0,0,0
};
short filt24_0_dcr[24] = {
  2730,5461,8192,10922,13653,16384,14043,11703,9362,7022,4681,0,0,0,0,0,0,0,0,0,0,0,0
};

short filt24_1[24] = {
  0,2730,5461,8192,10922,13653,16384,13653,10922,8192,5461,2730,0,0,0,0,0,0,0,0,0,0,0,0
};
short filt24_1_dcl[24] = {
  0,4681,7022,9362,11703,14043,16384,13653,10922,8192,5461,2730,0,0,0,0,0,0,0,0,0,0,0,0
};
short filt24_1_dcr[24] = {
  0,2730,5461,8192,10922,13653,16384,14043,11703,9362,7022,4681,0,0,0,0,0,0,0,0,0,0,0,0
};


short filt24_2[24] = {
  0,0,2730,5461,8192,10922,13653,16384,13653,10922,8192,5461,2730,0,0,0,0,0,0,0,0,0,0,0
};
short filt24_2_dcl[24] = {
  0,0,2341,4681,7022,9362, 11703,16384,13653,10922,8192,5461,2730,0,0,0,0,0,0,0,0,0,0,0
};
short filt24_2_dcr[24] = {
  0,0,2730,5461,8192,10922,13653,16384,14043,11703,9362,4681,2341,0,0,0,0,0,0,0,0,0,0,0
};

//  X X X Y | X X X X | X Y X X
short filt24_3[24] = {
  0,0,0,2730,5461,8192,10922,13653,16384,13653,10922,8192,5461,2730,0,0,0,0,0,0,0,0,0,0
};
short filt24_3_dcl[24] = {
  0,0,0,2341,4681,7022,9362,14043,16384,13653,10922,8192,5461,2730,0,0,0,0,0,0,0,0,0,0
};
//  X X X Y | X X DC X X | X Y X X
short filt24_3_dcr[24] = {
  0,0,0,2730,5461,8192,10922,13653,16384,14043,11703,7022,4681,2341,0,0,0,0,0,0,0,0,0,0
};


short filt24_4[24] = {
  0,0,0,0,2730,5461,8192,10922,13653,16384,13653,10922,8192,5461,2730,0,0,0,0,0,0,0,0,0
};
short filt24_4_dcl[24] = {
  0,0,0,0,2341,7022,9362,11703,14043,16384,13653,10922,8192,5461,2730,0,0,0,0,0,0,0,0,0
};
short filt24_4_dcr[24] = {
  0,0,0,0,2730,5461,8192,10922,13653,16384,14043,11703,7022,4681,2341,0,0,0,0,0,0,0,0,0
};

short filt24_5[24] = {
  0,0,0,0,0,2730,5461,8192,10922,13653,16384,13653,10922,8192,5461,2730,0,0,0,0,0,0,0,0
};
//  X X X Y | X X DC X X | X Y X X
short filt24_5_dcl[24] = {
  0,0,0,0,0,2341,4681,9362,11703,14043,16384,13653,10922,8192,5461,2730,0,0,0,0,0,0,0,0
};
short filt24_5_dcr[24] = {
  0,0,0,0,0,2730,5461,8192,10922,13653,16384,11703,9362,7022,4681,2730,0,0,0,0,0,0,0,0
};


short filt24_6[24] = {
  0,0,0,0,0,0,2730,5461,8192,10922,13653,16384,13653,10922,8192,5461,2730,0,0,0,0,0,0,0
};
short filt24_6_dcl[24] = {
  0,0,0,0,0,0,4681,7022,9362,11703,14043,16384,13653,10922,8192,5461,2730,0,0,0,0,0,0,0
};
short filt24_6_dcr[24] = {
  0,0,0,0,0,0,2730,5461,8192,10922,13653,16384,14043,11703,9362,7022,4681,0,0,0,0,0,0,0
};


short filt24_7[24] = {
  0,0,0,0,0,0,0,2730,5461,8192,10922,13653,16384,13653,10922,8192,5461,2730,0,0,0,0,0,0
};
short filt24_7_dcl[24] = {
  0,0,0,0,0,0,0,4681,7022,9362,11703,14043,16384,13653,10922,8192,5461,2730,0,0,0,0,0,0
};
short filt24_7_dcr[24] = {
  0,0,0,0,0,0,0,2730,5461,8192,10922,13653,16384,14043,11703,9362,7022,4681,0,0,0,0,0,0
};

short filt24_0l[24] = {
  30037,27306,24576,21845,19114,16384,13653,10922,8192,5461,2730,0,0,0,0,0,0,0,0,0,0,0,0,0
};
short filt24_1l[24] = {
  0,30037,27306,24576,21845,19114,16384,13653,10922,8192,5461,2730,0,0,0,0,0,0,0,0,0,0,0,0
};
short filt24_2l[24] = {
  0,0,30037,27306,24576,21845,19114,16384,13653,10922,8192,5461,2730,0,0,0,0,0,0,0,0,0,0,0
};
short filt24_3l[24] = {
  //0,0,0,30037,27306,24576,21845,19114,16384,13653,10922,8192,5461,2730,0,0,0,0,0,0,0,0,0,0};
  0,0,0,0,0,24576,21845,19114,16384,13653,10922,8192,5461,2730,0,0,0,0,0,0,0,0,0,0
};
short filt24_4l[24] = {
  0,0,0,0,30037,27306,24576,21845,19114,16384,13653,10922,8192,5461,2730,0,0,0,0,0,0,0,0,0
};
short filt24_5l[24] = {
  0,0,0,0,0,30037,27306,24576,21845,19114,16384,13653,10922,8192,5461,2730,0,0,0,0,0,0,0,0
};
short filt24_6l[24] = {
  0,0,0,0,0,0,30037,27306,24576,21845,19114,16384,13653,10922,8192,5461,2730,0,0,0,0,0,0,0
};
short filt24_7l[24] = {
  0,0,0,0,0,0,0,30037,27306,24576,21845,19114,16384,13653,10922,8192,5461,2730,0,0,0,0,0,0
};
short filt24_0l2[24] = {
  2730,5461,8192,10922,13653,16384,13653,10922,8192,5461,2730,0,0,0,0,0,0,0,0,0,0,0,0,0
};
short filt24_1l2[24] = {
  0,2730,5461,8192,10922,13653,16384,13653,10922,8192,5461,2730,0,0,0,0,0,0,0,0,0,0,0,0
};
short filt24_2l2[24] = {
  -2730,0,2730,5461,8192,10922,13653,16384,13653,10922,8192,5461,2730,0,0,0,0,0,0,0,0,0,0,0
};
short filt24_3l2[24] = {
  -5461,-2730,0,2730,5461,8192,10922,13653,16384,13653,10922,8192,5461,2730,0,0,0,0,0,0,0,0,0,0
};
short filt24_4l2[24] = {
  -8192,-5461,-2730,0,2730,5461,8192,10922,13653,16384,13653,10922,8192,5461,2730,0,0,0,0,0,0,0,0,0
};
short filt24_5l2[24] = {
  0,-8192,-5461,-2730,0,2730,5461,8192,10922,13653,16384,13653,10922,8192,5461,2730,0,0,0,0,0,0,0,0
};
short filt24_6l2[24] = {
  -13653,-10922,-8192,-5461,-2730,0,2730,5461,8192,10922,13653,16384,13653,10922,8192,5461,2730,0,0,0,0,0,0,0
};
short filt24_7l2[24] = {
  0,-13653,-10922,-8192,-5461,-2730,0,2730,5461,8192,10922,13653,16384,13653,10922,8192,5461,2730,0,0,0,0,0,0
};
short filt24_0r[24] = {
  2730,5461,8192,10922,13653,16384,19114,21845,24576,27306,30037,0,0,0,0,0,0,0,0,0,0,0,0,0
};
short filt24_1r[24] = {
  0,2730,5461,8192,10922,13653,16384,19114,21845,24576,27306,30037,0,0,0,0,0,0,0,0,0,0,0,0
};
short filt24_2r[24] = {
  0,0,2730,5461,8192,10922,13653,16384,19114,21845,24576,27306,30037,0,0,0,0,0,0,0,0,0,0,0
};
short filt24_3r[24] = {
  0,0,0,2730,5461,8192,10922,13653,16384,19114,21845,24576,27306,30037,0,0,0,0,0,0,0,0,0,0
};
short filt24_4r[24] = {
  0,0,0,0,2730,5461,8192,10922,13653,16384,19114,21845,24576,27306,30037,0,0,0,0,0,0,0,0,0
};
short filt24_5r[24] = {
  0,0,0,0,0,2730,5461,8192,10922,13653,16384,19114,21845,24576,27306,30037,0,0,0,0,0,0,0,0
};
short filt24_6r[24] = {
  0,0,0,0,0,0,2730,5461,8192,10922,13653,16384,19114,21845,24576,27306,30037,0,0,0,0,0,0,0
};
short filt24_7r[24] = {
  0,0,0,0,0,0,0,2730,5461,8192,10922,13653,16384,19114,21845,24576,27306,30037,0,0,0,0,0,0
};
short filt24_0r2[24] = {  /****/
  2730,5461,8192,10922,13653,16384,13653,10922,8192,5461,2730,0,-2730,-5461,-8192,-10922,-13653,0,0,0,0,0,0,0
};
short filt24_1r2[24] = {
  0,2730,5461,8192,10922,13653,16384,13653,10922,8192,5461,2730,0,-2730,-5461,-8192,-10922,-13653,0,0,0,0,0,0
};
short filt24_2r2[24] = {
  0,0,2730,5461,8192,10922,13653,16384,13653,10922,8192,5461,2730,0,-2730,-5461,-8192,-10922,-13653,0,0,0,0,0
};
short filt24_3r2[24] = {
  0,0,0,2730,5461,8192,10922,13653,16384,13653,10922,8192,5461,2730,0,-2730,-5461,-8192,-10922,-13653,0,0,0,0
};
short filt24_4r2[24] = {
  0,0,0,0,2730,5461,8192,10922,13653,16384,13653,10922,8192,5461,2730,0,-2730,-5461,-8192,-10922,-13653,0,0,0
};
short filt24_5r2[24] = {
  0,0,0,0,0,2730,5461,8192,10922,13653,16384,13653,10922,8192,5461,2730,0,-2730,-5461,-8192,-10922,-13653,0,0
};
short filt24_6r2[24] = {
  0,0,0,0,0,0,2730,5461,8192,10922,13653,16384,13653,10922,8192,5461,2730,0,-2730,-5461,-8192,-10922,-13653,0
};
short filt24_7r2[24] = {
  0,0,0,0,0,0,0,2730,5461,8192,10922,13653,16384,13653,10922,8192,5461,2730,0,-2730,-5461,-8192,-10922,-13653
};
