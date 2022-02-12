

#include "raw_dep.hpp"

id_type get_id(id_tuple tuple){
  id_type id =0;
  id = ((tuple.a * tuple.b) ^ ((tuple.c * tuple.d)>>5)) & (MEMORY_SIZE-1);
  return id;
}

data_type update_logic(data_type a, data_type b){
  #pragma HLS inline
  data_type result;
  #if OP_CODE == 1
    result = a * b;
    #if DATATYPE == 1
      constexpr unsigned int OP_LATENCY = MIN(UPDATE_LATENCY-1,9);
      #pragma HLS bind_op variable=result op=fmul latency=OP_LATENCY impl=fabric
    #elif DATATYPE == 2
      constexpr unsigned int OP_LATENCY = MIN(UPDATE_LATENCY-1,4);
      #pragma HLS bind_op variable=result op=mul latency=OP_LATENCY impl=fabric
    #else
      constexpr unsigned int OP_LATENCY = MIN(UPDATE_LATENCY-1,10);
      #pragma HLS bind_op variable=result op=dmul latency=OP_LATENCY impl=fabric
    #endif

  #elif OP_CODE == 2
    result = a / b;
    #if DATATYPE == 1
      constexpr unsigned int OP_LATENCY = MIN(UPDATE_LATENCY-1,29);
      #pragma HLS bind_op variable=result op=fdiv latency=OP_LATENCY impl=fabric
    // #elif DATATYPE == 2 (There is no div impl pragma)
    #else
      constexpr unsigned int OP_LATENCY = MIN(UPDATE_LATENCY-1,58);
      #pragma HLS bind_op variable=result op=ddiv latency=OP_LATENCY impl=fabric
    #endif
  #else
    result = a + b;
    #if DATATYPE == 1
      constexpr unsigned int OP_LATENCY = MIN(UPDATE_LATENCY-1,13);
      #pragma HLS bind_op variable=result op=fadd latency=OP_LATENCY impl=fabric
    #elif DATATYPE == 2
      constexpr unsigned int OP_LATENCY = MIN(UPDATE_LATENCY-1,4);
      #pragma HLS bind_op variable=result op=add latency=OP_LATENCY impl=fabric
    #else
      constexpr unsigned int OP_LATENCY = MIN(UPDATE_LATENCY-1,13);
      #pragma HLS bind_op variable=result op=dadd latency=OP_LATENCY impl=fabric
    #endif
  #endif
  return result;
}
