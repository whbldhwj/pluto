// This is automatically generated by PolySA
// Array type: SYNC

#include "Halide.h"
#include <iostream>

using namespace Halide;
using namespace std;

int main(void) {
// Input declarations
ImageParam B(type_of<int>(), 2, "B");
ImageParam A(type_of<int>(), 2, "A");
ImageParam C_ext(type_of<int>(), 3, "C_ext");
ImageParam C(type_of<int>(), 2, "C");

// Input initialization

// Variable declarations
Var t1, t2, t3;

// Function declarations
#define FUNC_S type_of<int>(), {t1, t2, t3}, Place::Host
Func B_CC0_E(FUNC_S), A_CC1_E(FUNC_S), C_CC3_E(FUNC_S), C_ext_CC2_I(FUNC_S), APP(FUNC_S);

// UREs
B_CC0_E(t1, t2, t3) = 0;
B_CC0_E(t1, t2, t3) = select((t2 - 1 == 0 && t1 - t3 - 3 >= 0) || (-t1 + t3 + 2 == 0 && t2 - 1 == 0), B(t1 - t2 - t3, t3), select((t2 - 2 >= 0 && t1 - t2 - t3 - 2 >= 0) || (-t1 + t2 + t3 + 1 == 0 && t2 - 2 >= 0), B_CC0_E(t1 - 1, t2 - 1, t3), B_CC0_E(t1, t2, t3)));
A_CC1_E(t1, t2, t3) = 0;
A_CC1_E(t1, t2, t3) = select((t3 - 1 == 0 && t1 - t2 - 3 >= 0) || (t3 - 1 == 0 && -t1 + t2 + 2 == 0), A(t2, t1 - t2 - t3), select((t1 - t2 - t3 - 2 >= 0 && t3 - 2 >= 0) || (-t1 + t2 + t3 + 1 == 0 && t1 - t2 - 3 >= 0), A_CC1_E(t1 - 1, t2, t3 - 1), A_CC1_E(t1, t2, t3)));
C_ext_CC2_I(t1, t2, t3) = 0;
C_ext_CC2_I(t1, t2, t3) = select((-t1 + t2 + t3 + 1 == 0), (A_CC1_E(t1, t2, t3) * B_CC0_E(t1, t2, t3)), C_ext_CC2_I(t1, t2, t3));
C_ext_CC2_I(t1, t2, t3) = select((t1 - t2 - t3 - 2 >= 0), (C_ext_CC2_I(t1 - 1, t2, t3) + (A_CC1_E(t1, t2, t3) * B_CC0_E(t1, t2, t3))), C_ext_CC2_I(t1, t2, t3));
C_CC3_E(t1, t2, t3) = 0;
C_CC3_E(t1, t2, t3) = select((-t1 + t2 + t3 + 8 == 0), C_ext_CC2_I(t1, t2, t3), C_CC3_E(t1, t2, t3));
APP(t1, t2, t3) = 0;
APP(t1, t2, t3) = C_CC3_E(t1, t2, t3);

// Build the initial loop nest
Var tloop1;
APP.merge_defs({B_CC0_E.update(0), A_CC1_E.update(0), C_ext_CC2_I.update(0), C_ext_CC2_I.update(1), C_CC3_E.update(0), APP.update(0)}, {B_CC0_E, A_CC1_E, C_ext_CC2_I, C_CC3_E})
   .reorder_inward(t1, t2, t3)
   .space_time_transform({t1, t2, t3},
                         {tloop1},
                         {t2, t3},
                         {1, 0, 0,
                          0, 1, 0,
                          0, 0, 1},
                         {1, 0, 0,
                          0, 1, 0,
                          0, 0, 1})
   .domain(t1, 3, 24, 1,
           t2, 1, 8, 1,
           t3, 1, 8, 1,
           tloop1, 3, 24, 1);

// PE Optimization

// CPU Realization
Image<int> FPGA_output(24 + 1, 8 + 1, 8 + 1);
APP.realize(FPGA_output);
cout << "END" << endl;

// CPU Verification

/*
// Build I/O network
Func B_CC0_E_serializer(Place::Host), B_CC0_E_loader, B_CC0_E_feeder, A_CC1_E_serializer(Place::Host), A_CC1_E_loader, A_CC1_E_feeder, C_ext_CC2_I_drainer, C_ext_CC2_I_collector, C_ext_CC2_I_unloader, C_ext_CC2_I_deserializer(Place::Host), C_CC3_E_drainer, C_CC3_E_collector, C_CC3_E_unloader, C_CC3_E_deserializer(Place::Host);
APP.isolate_consumer_chain(B_CC0_E, B_CC0_E_feeder, B_CC0_E_loader, B_CC0_E_serializer(Place::Host))
   .isolate_consumer_chain(A_CC1_E, A_CC1_E_feeder, A_CC1_E_loader, A_CC1_E_serializer(Place::Host))
   .isolate_producer_chain(C_ext_CC2_I, C_ext_CC2_I_drainer, C_ext_CC2_I_collector, C_ext_CC2_I_unloader C_ext_CC2_I_deserializer(Place::Host))
   .isolate_producer_chain(C_CC3_E, C_CC3_E_drainer, C_CC3_E_collector, C_CC3_E_unloader C_CC3_E_deserializer(Place::Host));

// I/O Optimization

*/
}
