

#include "../raw_dep.hpp"
#include <cstdlib>
#include <iostream>

constexpr double ERROR_TOL = 1E-8;
constexpr int TEST_ROUNDS = 3;

void raw_dep_sw(stream<packet_type> &in, stream<data_type> &out) {

  bool process = true;
  data_type memory[MEMORY_SIZE] = {0};


  for (unsigned i = 0; i < MEMORY_SIZE; ++i) {
	  memory[i] = MEM_INIT;
	}

  for (unsigned i = 0; i < NUM_OF_SAMPLES; ++i) {
    auto in_elem = in.read();

    address_type id = get_id(in_elem.tuple);
    #if OP_CODE == 1
      memory[id] *= in_elem.num;
    #elif OP_CODE == 2
      memory[id] /= in_elem.num;
    #else
      memory[id] += in_elem.num;
    #endif
  }

  // Memory flush
  for (unsigned i = 0; i < MEMORY_SIZE; ++i) {
    // if using BRAM in read-first mode, only one operation is needed
    auto elem = memory[i];
    memory[i] = MEM_INIT;
    out << elem;
  }
}

int main(int argc, char const *argv[]) {

  // Input generation
  stream<packet_type> hw_in, sw_in;
  stream<data_type> hw_out, sw_out;

  srand(0);
  for (unsigned round_id = 0; round_id < TEST_ROUNDS; ++round_id) {
    cout<<"Test round "<<round_id<<" | ";
    for (unsigned i = 0; i < NUM_OF_SAMPLES; ++i) {
      packet_type new_packet;
      new_packet.num = DATATYPE == 2? (data_type)rand() : ((data_type)rand())/RAND_MAX;
      #if OP_CODE == 2
        new_packet.num += 2;
      #endif
      if (round_id < 2) {
        // test all to the same dir
        new_packet.tuple.a = 0;
        new_packet.tuple.b = 0;
        new_packet.tuple.c = 0;
        new_packet.tuple.d = 0;
      } else{
        new_packet.tuple.a = rand();
        new_packet.tuple.b = rand();
        new_packet.tuple.c = rand();
        new_packet.tuple.d = rand();
      }

      hw_in << new_packet;
      sw_in << new_packet;
    }

    // DUT
    raw_dep(hw_in, hw_out);

    // SW version
    raw_dep_sw(sw_in, sw_out);

    // check
    ASSERT(hw_in.size(), ==, sw_in.size())

    int out_cnt = 0;
    while (!hw_out.empty()) {
      auto sw_elem = sw_out.read();
      auto hw_elem = hw_out.read();

      ASSERT(abs(hw_elem - sw_elem), <, ERROR_TOL,
             "Elem. num: " << out_cnt << " |hw_elem: " << hw_elem
                           << " |sw_elem: " << sw_elem)
      out_cnt++;
    }

    ASSERT(hw_out.size(), ==, 0)
    ASSERT(sw_out.size(), ==, 0)

    cout<<"Pass"<<endl;
  }

  return 0;
}
