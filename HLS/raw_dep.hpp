
#ifndef RAW_DEP_HPP
#define RAW_DEP_HPP

// HLS libs
#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <cinttypes>

/********************** Aux functions  ******************************/
#ifndef __SYNTHESIS__
  #define GET_ASSERT_MACRO(_1,_2,_3,_4,NAME,...) NAME
  #define ASSERT(...)  GET_ASSERT_MACRO(__VA_ARGS__,ASSERT4,ASSERT3,ASSERT2,ASSERT1)(__VA_ARGS__)
  #define ASSERT1(x) assert(x);
  // #define ASSERT2(x) assert(x) undefined
  #define ASSERT3(v1,comp,v2) \
  if(!(v1 comp v2)){ \
  std::cout<<"Test (" #v1 " " #comp " " #v2 ") Failed | " #v1 " = " <<v1 << " | " \
  << #v2 " = "<<v2 <<std::endl; \
  assert(v1 comp v2); \
  }

  #define ASSERT4(v1,comp,v2,text) \
  if(!(v1 comp v2)){ \
  std::cout<<text<< "| Test (" #v1 " " #comp " " #v2 ") Failed | " #v1 " = " <<v1 << " | " \
  << #v2 " = "<<v2 <<std::endl; \
  assert(v1 comp v2); \
  }

  #include <iostream>
  using namespace std;

  #define START_SW_ONLY_LOOP(cont_condition) while (cont_condition){
  #define END_SW_ONLY_LOOP }
#else
  #define START_SW_ONLY_LOOP(cont_condition)
  #define END_SW_ONLY_LOOP
  #define ASSERT(...)
#endif

#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

constexpr int floorlog2(int x){
  return  x == 0 ? -1 : x == 1 ? 0 : 1+floorlog2(x >> 1);
}

constexpr int ceillog2(int x){
  return x == 0 ? -1 :x == 1 ? 0 : floorlog2(x - 1) + 1;
}

#define NBITS2(n) ((n&2)?1:0)
#define NBITS4(n) ((n&(0xC))?(2+NBITS2(n>>2)):(NBITS2(n)))
#define NBITS8(n) ((n&0xF0)?(4+NBITS4(n>>4)):(NBITS4(n)))
#define NBITS16(n) ((n&0xFF00)?(8+NBITS8(n>>8)):(NBITS8(n)))
#define NBITS32(n) ((n&0xFFFF0000)?(16+NBITS16(n>>16)):(NBITS16(n)))
#define NBITS(n) (n==0?0:NBITS32(n)+1)

using namespace hls;

/******************* Parameter and type definitions ***************************/

constexpr unsigned int NUM_OF_SAMPLES = 1000;
constexpr unsigned int MEMORY_BITS = 8;
constexpr unsigned int MEMORY_SIZE = 1<<MEMORY_BITS;
constexpr unsigned int ID_EMPTY = 1<<MEMORY_BITS;


typedef ap_uint<MEMORY_BITS> address_type;

#ifndef ID_FACTOR
  #define ID_FACTOR 1 //0: Sum | 1:mul | 2:div
#endif
#define ID_BITS (MEMORY_BITS*ID_FACTOR)
typedef ap_uint<ID_BITS> id_type;

struct id_tuple {
  uint8_t a,b,c,d;
};

id_type get_id(id_tuple tuple);

#ifndef DD
  #define DD 1 //0: Sum | 1:mul | 2:div
#endif

#ifndef OP_CODE
  #define OP_CODE 0 //0: Sum | 1:mul | 2:div
#endif

#ifndef DATATYPE
  #define DATATYPE 0
#endif

#if DATATYPE == 1
  typedef float data_type;
#elif DATATYPE == 2
  typedef int64_t data_type;
#else
  typedef double data_type;
#endif

#if OP_CODE == 1
  constexpr data_type MEM_INIT = 1;
#elif OP_CODE == 2
  constexpr data_type MEM_INIT = 1<<20;
#else
  constexpr data_type MEM_INIT = 0;
#endif

constexpr unsigned int UPDATE_LATENCY = DD;


struct packet_type {
  id_tuple tuple;
  data_type num;
};

struct packet_type_id {
  ap_uint<1> valid;
  id_type id;
  data_type num;
};


/****************** function and classes declaration **************************/

data_type update_logic(data_type a, data_type b);

void raw_dep(stream<packet_type> &in, stream<data_type> &out );

template <typename T ,unsigned int DEPTH>
struct WaitList {
// public:
  T list[DEPTH]; // public so pragmas can be applied

  WaitList (T init_val = 0){
    for (unsigned int i = 0; i < DEPTH; i++) {
      #pragma HLS unroll
      list[i]= init_val;
    }
  };


  void update(T val) {
    for (unsigned int i = DEPTH-1; i > 0; i--) {
      #pragma HLS unroll
      list[i] = list[i-1];
    }
    list[0] = val;
  }

  bool is_in_list(T val) {
    bool found=false;
    for (unsigned int i = 0; i < DEPTH; i++) {
      #pragma HLS unroll
      found |= list[i] == val;
    }
    return found;
  }

  bool is_before_last(T val) {
    bool found=false;
    for (unsigned int i = 0; i < DEPTH-1; i++) {
      #pragma HLS unroll
      found |= list[i] == val;
    }
    return found;
  }

  bool is_front(T val) {
    return list[DEPTH-1] == val;
  }


};

#endif // RAW_DEP_HPP
