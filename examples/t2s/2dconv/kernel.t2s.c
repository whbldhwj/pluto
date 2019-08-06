// This is automatically generated by PolySA

// Input declarations
ImageParam W(type_of<float>(), 2, "W");
ImageParam X(type_of<float>(), 2, "X");
ImageParam Z_ext(type_of<float>(), 4, "Z_ext");

// Global setting
T2S.setting(LoopOrder::Inward, Place::Device);

// Function declarations
Func W_CC0_E, X_CC1_E, X_CC3_E, W_CC4_E, X_CC5_E, W_CC6_E, X_CC7_E, W_CC8_E, Z_ext_CC2_I, Z_ext_CC2_ID, APP;

// Variable declarations
Var t1, t2, t3, t4;

// UREs
W_CC0_E(t1, t2, t3, t4) = select((t4 == 0 && t3 == 0 && t2 == 0 && t1 >= 0 && -t1 + (31) >= 0), W(0, 0), W_CC0_E(t1 + (-1), t2, t3, t4));
X_CC1_E(t1, t2, t3, t4) = select((t4 == 0 && t2 == 0 && t1 >= 0 && -t1 + (31) >= 0 && t3 >= 0 && -t3 + (31) >= 0), X(t3, t1), X_CC1_E(t1, t2, t3, t4));
Z_ext_CC2_I(t3, t1, 0, 0) = select((t2 == 0 && t4 == 0 && t1 >= 0 && t3 >= 0 && -t1 + (31) >= 0 && -t3 + (31) >= 0), (X_CC1_E(t1, t2, t3, t4) * W_CC0_E(t1, t2, t3, t4)));
X_CC3_E(t1, t2, t3, t4) = select((t4 + (-2) == 0 && t2 + -1 == 0 && t1 >= 0 && -t1 + (31) >= 0 && t3 >= 0 && -t3 + (31) >= 0) || (t4 + (-3) == 0 && t3 + (-31) == 0 && t2 + (-2) == 0 && t1 >= 0 && -t1 + (31) >= 0), X(t2t3, t1), X_CC3_E(t1 + (1), t2, t3 + (-1), t4 + (-1)));
W_CC4_E(t1, t2, t3, t4) = select((-t2 + t4 + -1 == 0 && t1 == 0 && t3 >= 0 && -t3 + (31) >= 0 && t2 + -1 >= 0 && -t2 + (2) >= 0), W(t2, 0), W_CC4_E(t1, t2 + (-1), t3, t4));
Z_ext_CC2_I(t3, t1, t2, 0) = select((t2 + -t4 + 1 == 0 && t3 >= 0 && t1 >= 0 && -t4 + (3) >= 0 && -t3 + (31) >= 0 && -t1 + (31) >= 0 && t4 + (-2) >= 0), (Z_ext_CC2_I(t3, t1, t2 + (-1), (2)) + (X_CC3_E(t1, t2, t3, t4) * W_CC4_E(t1, t2, t3, t4))));
X_CC5_E(t1, t2, t3, t4) = select((t2 + -1 == 0 && t1 >= 0 && -t1 + (31) >= 0 && t4 + (-2) >= 0 && -t4 + (3) >= 0 && t3 >= 0 && -t3 + (31) >= 0) || (t3 + (-31) == 0 && t2 + (-2) == 0 && t1 >= 0 && -t1 + (31) >= 0 && t4 + (-3) >= 0 && -t4 + (4) >= 0), X(t2t3, t1(-1) * t2t4), X_CC5_E(t1 + (1), t2, t3 + (-1), t4 + (-1)));
W_CC6_E(t1, t2, t3, t4) = select((t3 == 0 && t1 >= 0 && -t1 + (31) >= 0 && t2 + -1 >= 0 && -t2 + (2) >= 0 && -t2 + t4 + -1 >= 0 && t2 + -t4 + (2) >= 0), W(t2, (-1) * t2t4), W_CC6_E(t1 + (-1), t2, t3, t4));
Z_ext_CC2_I(t3, t1, t2, (-1) * t2 + t4) = select((t2 + -t4 + (2) >= 0 && -t1 + (31) >= 0 && -t2 + (2) >= 0 && -t3 + (31) >= 0 && t2 + -1 >= 0 && t1 >= 0 && t3 >= 0 && -t2 + t4 + -1 >= 0), (Z_ext_CC2_I(t3, t1, t2, (-1) * t2 + t4 + (-1)) + (X_CC5_E(t1, t2, t3, t4) * W_CC6_E(t1, t2, t3, t4))));
X_CC7_E(t1, t2, t3, t4) = select((t2 == 0 && t1 >= 0 && -t1 + (31) >= 0 && t3 >= 0 && -t3 + (31) >= 0 && t4 + -1 >= 0 && -t4 + (2) >= 0), X(t3, t1t4), X_CC7_E(t1, t2, t3, t4));
W_CC8_E(t1, t2, t3, t4) = select((t2 == 0 && t1 == 0 && t4 + -1 >= 0 && -t4 + (2) >= 0 && t3 >= 0 && -t3 + (31) >= 0), W(0, t4), W_CC8_E(t1, t2 + (-1), t3, t4));
Z_ext_CC2_I(t3, t1, 0, t4) = select((t2 == 0 && t3 >= 0 && t1 >= 0 && -t4 + (2) >= 0 && -t3 + (31) >= 0 && -t1 + (31) >= 0 && t4 + -1 >= 0), (Z_ext_CC2_I(t3, t1, 0, t4 + (-1)) + (X_CC7_E(t1, t2, t3, t4) * W_CC8_E(t1, t2, t3, t4))));

// Build the initial loop nest
APP.merge_UREs(W_CC0_E, X_CC1_E, Z_ext_CC2_I, X_CC3_E, W_CC4_E, Z_ext_CC2_I.update(0), X_CC5_E, W_CC6_E, Z_ext_CC2_I.update(1), X_CC7_E, W_CC8_E, Z_ext_CC2_I.update(2))
   .domain(t1, 0, 31, 1,
           t2, 0, 2, 1,
           t3, 0, 31, 1,
           t4, 0, 2, 1);

// PE Optimization

// Space-time transformation
APP.space_time_transform({t1, t2, t3, t4},
                         {t1, t2},
                         {1, 0, 0, 0,
                          0, 1, 0, 0},
                         {t3, t4},
                         {0, 0, 1, 0,
                          0, 0, 0, 1},
                         Systolic::Async);

// Build I/O network
Func W_CC0_E_serializer(Place::Host), W_CC0_E_loader, W_CC0_E_feeder, X_CC1_E_serializer(Place::Host), X_CC1_E_loader, X_CC1_E_feeder, Z_ext_CC2_I_drainer, Z_ext_CC2_I_collector, Z_ext_CC2_I_unloader, Z_ext_CC2_I_deserializer(Place::Host), X_CC3_E_serializer(Place::Host), X_CC3_E_loader, X_CC3_E_feeder, W_CC4_E_serializer(Place::Host), W_CC4_E_loader, W_CC4_E_feeder, X_CC5_E_serializer(Place::Host), X_CC5_E_loader, X_CC5_E_feeder, W_CC6_E_serializer(Place::Host), W_CC6_E_loader, W_CC6_E_feeder, X_CC7_E_serializer(Place::Host), X_CC7_E_loader, X_CC7_E_feeder, W_CC8_E_serializer(Place::Host), W_CC8_E_loader, W_CC8_E_feeder;
APP.isolate_consumer_chain(W_CC0_E, W_CC0_E_feeder, W_CC0_E_loader, W_CC0_E_serializer)
   .isolate_consumer_chain(X_CC1_E, X_CC1_E_feeder, X_CC1_E_loader, X_CC1_E_serializer)
   .isolate_producer_chain(Z_ext_CC2_ID, Z_ext_CC2_I_drainer, Z_ext_CC2_I_collector, Z_ext_CC2_I_unloader Z_ext_CC2_I_deserializer)
   .isolate_consumer_chain(X_CC3_E, X_CC3_E_feeder, X_CC3_E_loader, X_CC3_E_serializer)
   .isolate_consumer_chain(W_CC4_E, W_CC4_E_feeder, W_CC4_E_loader, W_CC4_E_serializer)
   .isolate_consumer_chain(X_CC5_E, X_CC5_E_feeder, X_CC5_E_loader, X_CC5_E_serializer)
   .isolate_consumer_chain(W_CC6_E, W_CC6_E_feeder, W_CC6_E_loader, W_CC6_E_serializer)
   .isolate_consumer_chain(X_CC7_E, X_CC7_E_feeder, X_CC7_E_loader, X_CC7_E_serializer)
   .isolate_consumer_chain(W_CC8_E, W_CC8_E_feeder, W_CC8_E_loader, W_CC8_E_serializer);

// I/O Optimization

