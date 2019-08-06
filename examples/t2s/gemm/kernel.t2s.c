// This is automatically generated by PolySA

// Input declarations
ImageParam B(type_of<float>(), 2, "B");
ImageParam A(type_of<float>(), 2, "A");
ImageParam C_ext(type_of<float>(), 3, "C_ext");

// Global setting
T2S.setting(LoopOrder::Inward, Place::Device);

// Function declarations
Func B_CC0_E, A_CC1_E, B_CC3_E, A_CC4_E, C_ext_CC2_I, APP;

// Variable declarations
Var t1, t2, t3;

// UREs
B_CC0_E(t1, t2, t3) = select(t1 == 0 && t3 == 0 && t2 >= 0 && -t2 + (63) >= 0, B(0, t2), B_CC0_E(t1 + (-1), t2, t3));
A_CC1_E(t1, t2, t3) = select(t2 == 0 && t3 == 0 && t1 >= 0 && -t1 + (63) >= 0, A(t1, 0), A_CC1_E(t1, t2 + (-1), t3));
C_ext_CC2_I(t1, t2, 0) = select(t3 == 0 && t2 >= 0 && t1 >= 0 && -t2 + (63) >= 0 && -t1 + (63) >= 0, (A_CC1_E(t1, t2, t3) * B_CC0_E(t1, t2, t3)));
B_CC3_E(t1, t2, t3) = select(t1 == 0 && t2 >= 0 && -t3 + (63) >= 0 && -t2 + (63) >= 0 && t3 + -1 >= 0, B(t3, t2), B_CC3_E(t1 + (-1), t2, t3));
A_CC4_E(t1, t2, t3) = select(t2 == 0 && t1 >= 0 && -t3 + (63) >= 0 && -t1 + (63) >= 0 && t3 + -1 >= 0, A(t1, t3), A_CC4_E(t1, t2 + (-1), t3));
C_ext_CC2_I(t1, t2, t3) = select(t1 >= 0 && t2 >= 0 && -t3 + (63) >= 0 && -t1 + (63) >= 0 && -t2 + (63) >= 0 && t3 + -1 >= 0, (C_ext_CC2_I(t1, t2, t3 + (-1)) + (A_CC4_E(t1, t2, t3) * B_CC3_E(t1, t2, t3))));

// Build the initial loop nest
APP.merge_UREs(B_CC0_E, A_CC1_E, C_ext_CC2_I, B_CC3_E, A_CC4_E, C_ext_CC2_I.update(0))
   .domain(t1, 0, 63, 1,
           t2, 0, 63, 1,
           t3, 1, 63, 1);

// PE Optimization

// Space-time transformation
APP.space_time_transform({t1, t2, t3},
                         {t1, t2},
                         {1, 0, 0,
                          0, 1, 0},
                         {t3},
                         {0, 0, 1},
                         Systolic::Async);

// Build I/O network
Func B_CC0_E_serializer(Place::Host), B_CC0_E_loader, B_CC0_E_feeder, A_CC1_E_serializer(Place::Host), A_CC1_E_loader, A_CC1_E_feeder, C_ext_CC2_I_serializer(Place::Host), C_ext_CC2_I_loader, C_ext_CC2_I_feeder, B_CC3_E_serializer(Place::Host), B_CC3_E_loader, B_CC3_E_feeder, A_CC4_E_serializer(Place::Host), A_CC4_E_loader, A_CC4_E_feeder;
APP.isolate_consumer_chain(B_CC0_E, B_CC0_E_feeder, B_CC0_E_loader, B_CC0_E_serializer)
   .isolate_consumer_chain(A_CC1_E, A_CC1_E_feeder, A_CC1_E_loader, A_CC1_E_serializer)
   .isolate_consumer_chain(C_ext_CC2_I, C_ext_CC2_I_feeder, C_ext_CC2_I_loader, C_ext_CC2_I_serializer)
   .isolate_consumer_chain(B_CC3_E, B_CC3_E_feeder, B_CC3_E_loader, B_CC3_E_serializer)
   .isolate_consumer_chain(A_CC4_E, A_CC4_E_feeder, A_CC4_E_loader, A_CC4_E_serializer);

// I/O Optimization

