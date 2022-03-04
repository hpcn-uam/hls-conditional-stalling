

#include "../raw_dep.hpp"

void get_id_stage(stream<packet_type> &in, stream<packet_type_id> &out ){
  packet_type_id out_elem;
  auto in_elem = in.read();

  out_elem.id = get_id(in_elem.tuple);
  out_elem.num = in_elem.num;
  out << out_elem;
}

void raw_dep_core(stream<packet_type_id> &in, stream<data_type> &out ){
  static data_type memory[MEMORY_SIZE];
  #pragma HLS BIND_STORAGE variable=memory type=ram_s2p impl=bram latency=1
  static bool init_mem = true;
  #pragma HLS reset variable=init_mem

  if (init_mem) {
    init_loop:for (unsigned i = 0; i < MEMORY_SIZE; i++) {
      memory[i] = MEM_INIT;
    }
  }
  init_mem = false;

  update: for(unsigned i = 0; i < NUM_OF_SAMPLES; ++i) {
    #pragma HLS PIPELINE
    auto in_elem = in.read();
    address_type id = in_elem.id;
    {
      #pragma HLS LATENCY min=UPDATE_LATENCY max=UPDATE_LATENCY
      memory[id] = update_logic(memory[id],in_elem.num);
    }

  }

  // Memory flush
  flush: for(unsigned i = 0; i < MEMORY_SIZE; ++i) {
   // if using BRAM in read-first mode, only one operation is needed
    auto elem = memory[i];
    memory[i] = MEM_INIT;
    out<< elem;
  }

}


void raw_dep(stream<packet_type> &in, stream<data_type> &out ){
  stream<packet_type_id> packet_with_id;
  START_SW_ONLY_LOOP(! in.empty())
    get_id_stage(in, packet_with_id );
  END_SW_ONLY_LOOP
  raw_dep_core(packet_with_id, out );
}
