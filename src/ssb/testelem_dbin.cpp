#include "ssb_utils.h"
#include <iostream>
#include <bitset>
using namespace std;

int decodeElement(int i, uint* data_block) {
  // Reference for the frame
  int reference = reinterpret_cast<int*>(data_block)[0];

  cout << "reference: " << reference << endl;

  // Index of miniblock containing i
  uint miniblock_index = i/32;

  // Miniblock bitwidths
  uint miniblock_bitwidths = data_block[1];

  // Miniblock offset into data_block array
  uint miniblock_offset = 0;
  for (int j=0; j<miniblock_index; j++) {
    miniblock_offset += (miniblock_bitwidths & 255);
    miniblock_bitwidths >>= 8;
  }

  // Miniblock bitwidth
  uint bitwidth = miniblock_bitwidths & 255;

  // Entry index in the miniblock
  uint index_into_miniblock = i & (32 - 1);

  uint start_bitindex = (bitwidth * index_into_miniblock);
  uint start_intindex = 2 + start_bitindex/32;

  /*uint* data_block_uint = reinterpret_cast<uint*>(data_block);*/
  unsigned long long element_block = (((unsigned long long)data_block[miniblock_offset + start_intindex + 1]) << 32) | data_block[miniblock_offset + start_intindex];
  start_bitindex = start_bitindex & (32-1);

  uint element = (element_block & (((1LL<<bitwidth) - 1LL) << start_bitindex)) >> start_bitindex;

  return reference + element;
}

int main(int argc, char** argv) {
  string column_name = "lo_quantity";
  int *h_col_orig = loadColumn<int>(column_name, LO_LEN);

  encoded_column col = loadEncodedColumn(column_name, "dbin", LO_LEN);
  //int i = 119994368;
  int i = 0;

  if (argc == 2) {
    i = atoi(argv[1]);
  } else {
    cout << "Need i" << endl;
    exit(1);
  }

  int threadIdx = i % 128;
  int block_index = i / 128;

  cout << "ThreadIdx " << threadIdx << " block_index " << block_index << endl;

  uint block_start = col.block_start[block_index];
  uint block_end = col.block_start[block_index + 1];

  {
    cout << "Block starts:" << endl;
    for (int i=0; i<5; i++) cout << col.block_start[block_index + i] << endl;

    cout << "Col values:" << endl;
    for (int i=block_index*128; i<block_index*128+128; i++) cout << h_col_orig[i] << ",";
    cout << endl;

    cout << "Binary:" << endl;
    for (int i=block_start-1; i<block_end; i++) cout << bitset<32>(col.data[i]) << endl;
  }

  // Shared memory for 4 blocks of encoded l_shipdate data
  uint* data_block = &col.data[block_start];

  int p = col.block_start[(block_index / 4) * 4];
  p--;
  int del_start = col.data[p];

  int item = decodeElement(threadIdx, data_block);
  cout << "Item Decoded: " << del_start + item << endl;
  cout << "Item Original: " << h_col_orig[i] << endl;

  return 0;
}

