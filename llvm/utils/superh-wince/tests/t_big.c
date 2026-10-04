#include "rt.h"

volatile u32 sink;

__attribute__((noinline)) u32 big(u32 x, u32 n) {
  u32 acc = 0;
  for (u32 i = 0; i < n; i++) {
    if (x & 1) {
      acc = (acc + 0x52e6b438u) * 3u + (x >> 0);
      acc = (acc - 0x6513270eu) * 3u + (x >> 1);
      acc = (acc + 0x0c5c7fd0u) * 3u + (x >> 2);
      acc = (acc ^ 0x1818e811u) * 3u + (x >> 3);
      acc = (acc - 0x0ed90475u) * 3u + (x >> 4);
      acc = (acc + 0x36f675ccu) * 3u + (x >> 5);
      acc = (acc ^ 0x1600a35au) * 3u + (x >> 6);
      acc = (acc + 0x6b0d549bu) * 3u + (x >> 7);
      acc = (acc + 0x3d9c1724u) * 3u + (x >> 8);
      acc = (acc + 0x6cad4a26u) * 3u + (x >> 9);
      acc = (acc + 0x1fb17c23u) * 3u + (x >> 10);
      acc = (acc - 0x0fd630f1u) * 3u + (x >> 11);
      acc = (acc + 0x658cda14u) * 3u + (x >> 12);
      acc = (acc + 0x3898d190u) * 3u + (x >> 13);
      acc = (acc ^ 0x2217beadu) * 3u + (x >> 14);
      acc = (acc + 0x6b4cb242u) * 3u + (x >> 15);
      acc = (acc - 0x1e27a1c0u) * 3u + (x >> 16);
      acc = (acc - 0x4ef8aa38u) * 3u + (x >> 17);
      acc = (acc + 0x2e44158bu) * 3u + (x >> 18);
      acc = (acc ^ 0x301850c5u) * 3u + (x >> 19);
      acc = (acc - 0x18f135d2u) * 3u + (x >> 20);
      acc = (acc - 0x1012f037u) * 3u + (x >> 21);
      acc = (acc - 0x0f4205b4u) * 3u + (x >> 22);
      acc = (acc ^ 0x34b9b5dfu) * 3u + (x >> 23);
      acc = (acc ^ 0x6d76b07eu) * 3u + (x >> 24);
      acc = (acc - 0x7731af10u) * 3u + (x >> 25);
      acc = (acc ^ 0x7403e430u) * 3u + (x >> 26);
      acc = (acc + 0x4cbd87adu) * 3u + (x >> 27);
      acc = (acc - 0x2e05319au) * 3u + (x >> 28);
      acc = (acc + 0x3e7d1bfbu) * 3u + (x >> 29);
      acc = (acc - 0x4cdd2055u) * 3u + (x >> 30);
      acc = (acc ^ 0x7ebff206u) * 3u + (x >> 0);
      acc = (acc ^ 0x72e6cc3au) * 3u + (x >> 1);
      acc = (acc + 0x12bd4aceu) * 3u + (x >> 2);
      acc = (acc + 0x6b0a18e8u) * 3u + (x >> 3);
      acc = (acc + 0x5790f82eu) * 3u + (x >> 4);
      acc = (acc ^ 0x7d2caf82u) * 3u + (x >> 5);
      acc = (acc - 0x0a097c97u) * 3u + (x >> 6);
      acc = (acc - 0x13deef86u) * 3u + (x >> 7);
      acc = (acc ^ 0x5051c1ccu) * 3u + (x >> 8);
      acc = (acc - 0x59a54a7bu) * 3u + (x >> 9);
      acc = (acc - 0x7f26144bu) * 3u + (x >> 10);
      acc = (acc + 0x74c9df6au) * 3u + (x >> 11);
      acc = (acc ^ 0x17f5e837u) * 3u + (x >> 12);
      acc = (acc - 0x795e8229u) * 3u + (x >> 13);
      acc = (acc + 0x10a3d6b2u) * 3u + (x >> 14);
      acc = (acc - 0x4f426dcbu) * 3u + (x >> 15);
      acc = (acc ^ 0x72158370u) * 3u + (x >> 16);
      acc = (acc - 0x62c33a4fu) * 3u + (x >> 17);
      acc = (acc + 0x58d5563du) * 3u + (x >> 18);
      acc = (acc ^ 0x7631a992u) * 3u + (x >> 19);
      acc = (acc - 0x2b0537e6u) * 3u + (x >> 20);
      acc = (acc ^ 0x1df9fd78u) * 3u + (x >> 21);
      acc = (acc + 0x0f17a300u) * 3u + (x >> 22);
      acc = (acc + 0x49952399u) * 3u + (x >> 23);
      acc = (acc ^ 0x3f63af83u) * 3u + (x >> 24);
      acc = (acc ^ 0x6415479cu) * 3u + (x >> 25);
      acc = (acc + 0x14a0f9e7u) * 3u + (x >> 26);
      acc = (acc ^ 0x72fdf202u) * 3u + (x >> 27);
      acc = (acc + 0x4720771fu) * 3u + (x >> 28);
      acc = (acc - 0x6e36aab0u) * 3u + (x >> 29);
      acc = (acc - 0x47469a4du) * 3u + (x >> 30);
      acc = (acc ^ 0x6a50df4du) * 3u + (x >> 0);
      acc = (acc + 0x616499c9u) * 3u + (x >> 1);
      acc = (acc + 0x26a2c0bdu) * 3u + (x >> 2);
      acc = (acc + 0x2d1c9af0u) * 3u + (x >> 3);
      acc = (acc - 0x3b618676u) * 3u + (x >> 4);
      acc = (acc + 0x3bbbe9eau) * 3u + (x >> 5);
      acc = (acc - 0x7c26847fu) * 3u + (x >> 6);
      acc = (acc ^ 0x2eae05cfu) * 3u + (x >> 7);
      acc = (acc + 0x482c9cbcu) * 3u + (x >> 8);
      acc = (acc ^ 0x254b0c4eu) * 3u + (x >> 9);
      acc = (acc - 0x5e8766edu) * 3u + (x >> 10);
      acc = (acc + 0x519088f5u) * 3u + (x >> 11);
      acc = (acc ^ 0x0dd27a65u) * 3u + (x >> 12);
      acc = (acc ^ 0x6472f1a3u) * 3u + (x >> 13);
      acc = (acc ^ 0x66237a04u) * 3u + (x >> 14);
      acc = (acc ^ 0x1a81682cu) * 3u + (x >> 15);
      acc = (acc + 0x66836886u) * 3u + (x >> 16);
      acc = (acc + 0x30cbc97du) * 3u + (x >> 17);
      acc = (acc ^ 0x3571810au) * 3u + (x >> 18);
      acc = (acc + 0x298cb3a5u) * 3u + (x >> 19);
      acc = (acc - 0x570dc195u) * 3u + (x >> 20);
      acc = (acc + 0x0d75985du) * 3u + (x >> 21);
      acc = (acc - 0x000f49c8u) * 3u + (x >> 22);
      acc = (acc - 0x26b94c7fu) * 3u + (x >> 23);
      acc = (acc ^ 0x19f9919cu) * 3u + (x >> 24);
      acc = (acc + 0x068739fau) * 3u + (x >> 25);
      acc = (acc - 0x353c631cu) * 3u + (x >> 26);
      acc = (acc + 0x6050914au) * 3u + (x >> 27);
      acc = (acc ^ 0x4093f6deu) * 3u + (x >> 28);
      acc = (acc ^ 0x5d39d0a8u) * 3u + (x >> 29);
      acc = (acc + 0x1f7296abu) * 3u + (x >> 30);
      acc = (acc ^ 0x7cf20724u) * 3u + (x >> 0);
      acc = (acc ^ 0x7afb2c68u) * 3u + (x >> 1);
      acc = (acc + 0x4fd58dbeu) * 3u + (x >> 2);
      acc = (acc + 0x24e4e25au) * 3u + (x >> 3);
      acc = (acc - 0x57b6fb7eu) * 3u + (x >> 4);
      acc = (acc ^ 0x43c71b9au) * 3u + (x >> 5);
      acc = (acc - 0x29540a6eu) * 3u + (x >> 6);
      acc = (acc + 0x05e999f3u) * 3u + (x >> 7);
      acc = (acc + 0x5c9bcf35u) * 3u + (x >> 8);
      acc = (acc - 0x06ec41adu) * 3u + (x >> 9);
      acc = (acc - 0x4c4f9b06u) * 3u + (x >> 10);
      acc = (acc - 0x174c77a2u) * 3u + (x >> 11);
      acc = (acc - 0x42d87208u) * 3u + (x >> 12);
      acc = (acc + 0x5de00997u) * 3u + (x >> 13);
      acc = (acc + 0x5b0ee76fu) * 3u + (x >> 14);
      acc = (acc - 0x5464ecc2u) * 3u + (x >> 15);
      acc = (acc - 0x39194242u) * 3u + (x >> 16);
      acc = (acc + 0x31f51707u) * 3u + (x >> 17);
      acc = (acc - 0x66934036u) * 3u + (x >> 18);
      acc = (acc + 0x3a0b9965u) * 3u + (x >> 19);
      acc = (acc ^ 0x7e26f36au) * 3u + (x >> 20);
      acc = (acc + 0x076b3e36u) * 3u + (x >> 21);
      acc = (acc ^ 0x4787f93bu) * 3u + (x >> 22);
      acc = (acc + 0x42594052u) * 3u + (x >> 23);
      acc = (acc ^ 0x5822cb77u) * 3u + (x >> 24);
      acc = (acc ^ 0x597a1ecfu) * 3u + (x >> 25);
      acc = (acc + 0x149e259bu) * 3u + (x >> 26);
      acc = (acc + 0x1a26f889u) * 3u + (x >> 27);
      acc = (acc + 0x78572976u) * 3u + (x >> 28);
      acc = (acc + 0x5675f6adu) * 3u + (x >> 29);
      acc = (acc - 0x7b8f2ab5u) * 3u + (x >> 30);
      acc = (acc ^ 0x007d1034u) * 3u + (x >> 0);
      acc = (acc - 0x5810d60eu) * 3u + (x >> 1);
      acc = (acc - 0x15b40aebu) * 3u + (x >> 2);
      acc = (acc ^ 0x1eb20109u) * 3u + (x >> 3);
      acc = (acc ^ 0x330698a1u) * 3u + (x >> 4);
      acc = (acc ^ 0x2db3997fu) * 3u + (x >> 5);
      acc = (acc + 0x551fd8f9u) * 3u + (x >> 6);
      acc = (acc ^ 0x6555abfeu) * 3u + (x >> 7);
      acc = (acc - 0x66c1494eu) * 3u + (x >> 8);
      acc = (acc - 0x15bd448fu) * 3u + (x >> 9);
      acc = (acc + 0x28aaca51u) * 3u + (x >> 10);
      acc = (acc + 0x20859634u) * 3u + (x >> 11);
      acc = (acc - 0x26b1cffcu) * 3u + (x >> 12);
      acc = (acc - 0x77216e9eu) * 3u + (x >> 13);
      acc = (acc - 0x256badf9u) * 3u + (x >> 14);
      acc = (acc - 0x796f74adu) * 3u + (x >> 15);
      acc = (acc + 0x59b44e92u) * 3u + (x >> 16);
      acc = (acc + 0x2188287eu) * 3u + (x >> 17);
      acc = (acc - 0x03a56cc1u) * 3u + (x >> 18);
      acc = (acc - 0x1a4f44f9u) * 3u + (x >> 19);
      acc = (acc ^ 0x23a5ef88u) * 3u + (x >> 20);
      acc = (acc + 0x31dec4f4u) * 3u + (x >> 21);
      acc = (acc ^ 0x072a98d2u) * 3u + (x >> 22);
      acc = (acc ^ 0x3678bc8du) * 3u + (x >> 23);
      acc = (acc - 0x3d93fd4cu) * 3u + (x >> 24);
      acc = (acc ^ 0x53740902u) * 3u + (x >> 25);
      acc = (acc + 0x6b446806u) * 3u + (x >> 26);
      acc = (acc - 0x0f977044u) * 3u + (x >> 27);
      acc = (acc ^ 0x5a9196f0u) * 3u + (x >> 28);
      acc = (acc - 0x6bae4b5bu) * 3u + (x >> 29);
      acc = (acc - 0x2179b37du) * 3u + (x >> 30);
      acc = (acc - 0x26debfdbu) * 3u + (x >> 0);
      acc = (acc ^ 0x04c9d78du) * 3u + (x >> 1);
      acc = (acc - 0x2ee0289du) * 3u + (x >> 2);
      acc = (acc + 0x0101b811u) * 3u + (x >> 3);
      acc = (acc + 0x2c1eea1fu) * 3u + (x >> 4);
      acc = (acc - 0x7936d536u) * 3u + (x >> 5);
      acc = (acc - 0x1ece615du) * 3u + (x >> 6);
      acc = (acc ^ 0x0fcf31cau) * 3u + (x >> 7);
      acc = (acc + 0x7b8444d1u) * 3u + (x >> 8);
      acc = (acc + 0x0e8bec94u) * 3u + (x >> 9);
      acc = (acc ^ 0x30f97058u) * 3u + (x >> 10);
      acc = (acc + 0x0acd8be1u) * 3u + (x >> 11);
      acc = (acc - 0x73c1cd2cu) * 3u + (x >> 12);
      acc = (acc + 0x072235c2u) * 3u + (x >> 13);
      acc = (acc ^ 0x7178ba0au) * 3u + (x >> 14);
      acc = (acc - 0x330c16a3u) * 3u + (x >> 15);
      acc = (acc ^ 0x46f5a1b4u) * 3u + (x >> 16);
      acc = (acc - 0x7a609683u) * 3u + (x >> 17);
      acc = (acc - 0x3f665edeu) * 3u + (x >> 18);
      acc = (acc - 0x4274a3ebu) * 3u + (x >> 19);
      acc = (acc ^ 0x33dcd77fu) * 3u + (x >> 20);
      acc = (acc ^ 0x231b3e14u) * 3u + (x >> 21);
      acc = (acc ^ 0x1f229dd0u) * 3u + (x >> 22);
      acc = (acc ^ 0x712ea6b3u) * 3u + (x >> 23);
      acc = (acc - 0x12926185u) * 3u + (x >> 24);
      acc = (acc ^ 0x3d9a8079u) * 3u + (x >> 25);
      acc = (acc + 0x12b80aedu) * 3u + (x >> 26);
      acc = (acc + 0x4d82feacu) * 3u + (x >> 27);
      acc = (acc - 0x2789d059u) * 3u + (x >> 28);
      acc = (acc + 0x5dbe3023u) * 3u + (x >> 29);
      acc = (acc + 0x40cbacd0u) * 3u + (x >> 30);
      acc = (acc + 0x77bd891fu) * 3u + (x >> 0);
      acc = (acc ^ 0x18189af4u) * 3u + (x >> 1);
      acc = (acc + 0x7cbd1f5au) * 3u + (x >> 2);
      acc = (acc + 0x3945336bu) * 3u + (x >> 3);
      acc = (acc - 0x6e7836a4u) * 3u + (x >> 4);
      acc = (acc ^ 0x67601367u) * 3u + (x >> 5);
      acc = (acc + 0x6bd8c676u) * 3u + (x >> 6);
      acc = (acc ^ 0x5b4b1b75u) * 3u + (x >> 7);
      acc = (acc - 0x179a071eu) * 3u + (x >> 8);
      acc = (acc + 0x5daf106du) * 3u + (x >> 9);
      acc = (acc - 0x5685d624u) * 3u + (x >> 10);
      acc = (acc ^ 0x756b7289u) * 3u + (x >> 11);
      acc = (acc ^ 0x04a10547u) * 3u + (x >> 12);
      acc = (acc - 0x54dd0ba5u) * 3u + (x >> 13);
      acc = (acc - 0x4ba2e161u) * 3u + (x >> 14);
      acc = (acc + 0x10755c97u) * 3u + (x >> 15);
      acc = (acc + 0x3a828159u) * 3u + (x >> 16);
      acc = (acc ^ 0x15850a03u) * 3u + (x >> 17);
      acc = (acc + 0x459c945cu) * 3u + (x >> 18);
      acc = (acc ^ 0x2e7a26e9u) * 3u + (x >> 19);
      acc = (acc ^ 0x212a8d9bu) * 3u + (x >> 20);
      acc = (acc ^ 0x42343354u) * 3u + (x >> 21);
      acc = (acc - 0x263cfa5eu) * 3u + (x >> 22);
      acc = (acc - 0x7e9ee51du) * 3u + (x >> 23);
      acc = (acc + 0x53b97377u) * 3u + (x >> 24);
      acc = (acc + 0x4770a087u) * 3u + (x >> 25);
      acc = (acc ^ 0x2eefa279u) * 3u + (x >> 26);
      acc = (acc ^ 0x1289bafau) * 3u + (x >> 27);
      acc = (acc - 0x044f1574u) * 3u + (x >> 28);
      acc = (acc ^ 0x16ac4191u) * 3u + (x >> 29);
      acc = (acc - 0x1570266bu) * 3u + (x >> 30);
      acc = (acc + 0x38efbaebu) * 3u + (x >> 0);
      acc = (acc + 0x43b30f66u) * 3u + (x >> 1);
      acc = (acc + 0x742a8063u) * 3u + (x >> 2);
      acc = (acc - 0x56d2a68cu) * 3u + (x >> 3);
      acc = (acc ^ 0x6af25748u) * 3u + (x >> 4);
      acc = (acc + 0x2114e068u) * 3u + (x >> 5);
      acc = (acc + 0x3d0a270bu) * 3u + (x >> 6);
      acc = (acc ^ 0x2954ba5cu) * 3u + (x >> 7);
      acc = (acc + 0x0ce5af69u) * 3u + (x >> 8);
      acc = (acc ^ 0x33a71568u) * 3u + (x >> 9);
      acc = (acc - 0x4e14d571u) * 3u + (x >> 10);
      acc = (acc ^ 0x34b3ff60u) * 3u + (x >> 11);
      acc = (acc - 0x721888ffu) * 3u + (x >> 12);
      acc = (acc ^ 0x2d8ad8c0u) * 3u + (x >> 13);
      acc = (acc + 0x58d50f1bu) * 3u + (x >> 14);
      acc = (acc + 0x401d68fbu) * 3u + (x >> 15);
      acc = (acc + 0x03edb920u) * 3u + (x >> 16);
      acc = (acc - 0x30803889u) * 3u + (x >> 17);
      acc = (acc + 0x7989e9d0u) * 3u + (x >> 18);
      acc = (acc + 0x72723b9cu) * 3u + (x >> 19);
      acc = (acc - 0x6ea330a1u) * 3u + (x >> 20);
      acc = (acc - 0x7eb86c57u) * 3u + (x >> 21);
      acc = (acc - 0x64a149f5u) * 3u + (x >> 22);
      acc = (acc - 0x4ecadea2u) * 3u + (x >> 23);
      acc = (acc + 0x37161c16u) * 3u + (x >> 24);
      acc = (acc + 0x57bb7d97u) * 3u + (x >> 25);
      acc = (acc ^ 0x23c49caeu) * 3u + (x >> 26);
      acc = (acc + 0x58f92deau) * 3u + (x >> 27);
      acc = (acc + 0x213bca7fu) * 3u + (x >> 28);
      acc = (acc - 0x121ae3e6u) * 3u + (x >> 29);
      acc = (acc ^ 0x416e99b0u) * 3u + (x >> 30);
      acc = (acc + 0x29ca862du) * 3u + (x >> 0);
      acc = (acc - 0x15a0cce6u) * 3u + (x >> 1);
      acc = (acc - 0x618177ffu) * 3u + (x >> 2);
      acc = (acc - 0x482cc78eu) * 3u + (x >> 3);
      acc = (acc - 0x3e01aaa6u) * 3u + (x >> 4);
      acc = (acc + 0x4b05e1aeu) * 3u + (x >> 5);
      acc = (acc + 0x759eb559u) * 3u + (x >> 6);
      acc = (acc ^ 0x28541424u) * 3u + (x >> 7);
      acc = (acc + 0x72218fdcu) * 3u + (x >> 8);
      acc = (acc ^ 0x4363e5d9u) * 3u + (x >> 9);
      acc = (acc - 0x54348156u) * 3u + (x >> 10);
      acc = (acc + 0x52d31e1bu) * 3u + (x >> 11);
      acc = (acc ^ 0x08d18011u) * 3u + (x >> 12);
      acc = (acc ^ 0x37c60e98u) * 3u + (x >> 13);
      acc = (acc + 0x2ed65411u) * 3u + (x >> 14);
      acc = (acc ^ 0x55d85e8du) * 3u + (x >> 15);
      acc = (acc ^ 0x1579da0au) * 3u + (x >> 16);
      acc = (acc - 0x4767e1fau) * 3u + (x >> 17);
      acc = (acc + 0x33736dccu) * 3u + (x >> 18);
      acc = (acc + 0x0144702bu) * 3u + (x >> 19);
      acc = (acc + 0x43a08f06u) * 3u + (x >> 20);
      acc = (acc ^ 0x24d4589cu) * 3u + (x >> 21);
      acc = (acc ^ 0x0aaaaf81u) * 3u + (x >> 22);
      acc = (acc ^ 0x05c22d3fu) * 3u + (x >> 23);
      acc = (acc - 0x4de2f8adu) * 3u + (x >> 24);
      acc = (acc + 0x3b996870u) * 3u + (x >> 25);
      acc = (acc - 0x27be9ab1u) * 3u + (x >> 26);
      acc = (acc ^ 0x63b759f5u) * 3u + (x >> 27);
      acc = (acc + 0x7e834904u) * 3u + (x >> 28);
      acc = (acc - 0x48bfcbcfu) * 3u + (x >> 29);
      acc = (acc + 0x250e7b34u) * 3u + (x >> 30);
      acc = (acc - 0x6de2fb1fu) * 3u + (x >> 0);
      acc = (acc - 0x23a9a9dau) * 3u + (x >> 1);
      acc = (acc - 0x041dcd94u) * 3u + (x >> 2);
      acc = (acc + 0x3add6527u) * 3u + (x >> 3);
      acc = (acc + 0x07fa22f7u) * 3u + (x >> 4);
      acc = (acc - 0x22126540u) * 3u + (x >> 5);
      acc = (acc + 0x5c57532bu) * 3u + (x >> 6);
      acc = (acc ^ 0x606a0debu) * 3u + (x >> 7);
      acc = (acc - 0x0cfff054u) * 3u + (x >> 8);
      acc = (acc - 0x04d2be09u) * 3u + (x >> 9);
      acc = (acc ^ 0x3e9b768fu) * 3u + (x >> 10);
      acc = (acc + 0x4387ee7bu) * 3u + (x >> 11);
      acc = (acc + 0x74fa9412u) * 3u + (x >> 12);
      acc = (acc - 0x1789819fu) * 3u + (x >> 13);
      acc = (acc - 0x10e8ad01u) * 3u + (x >> 14);
      acc = (acc ^ 0x794ec926u) * 3u + (x >> 15);
      acc = (acc ^ 0x130f27b2u) * 3u + (x >> 16);
      acc = (acc - 0x3c1ae917u) * 3u + (x >> 17);
      acc = (acc + 0x348922d7u) * 3u + (x >> 18);
      acc = (acc ^ 0x75d8d8a4u) * 3u + (x >> 19);
      acc = (acc + 0x61ef7bd1u) * 3u + (x >> 20);
      acc = (acc - 0x7aa068f1u) * 3u + (x >> 21);
      acc = (acc + 0x498dbfa8u) * 3u + (x >> 22);
      acc = (acc + 0x32c32444u) * 3u + (x >> 23);
      acc = (acc ^ 0x25bda659u) * 3u + (x >> 24);
      acc = (acc - 0x41023aedu) * 3u + (x >> 25);
      acc = (acc - 0x4dee4812u) * 3u + (x >> 26);
      acc = (acc + 0x222930aeu) * 3u + (x >> 27);
      acc = (acc + 0x7b7fec4bu) * 3u + (x >> 28);
      acc = (acc ^ 0x7c5d42dcu) * 3u + (x >> 29);
      acc = (acc - 0x197a14e2u) * 3u + (x >> 30);
      acc = (acc - 0x37bac233u) * 3u + (x >> 0);
      acc = (acc ^ 0x7d575d17u) * 3u + (x >> 1);
      acc = (acc ^ 0x491961a1u) * 3u + (x >> 2);
      acc = (acc ^ 0x774510cau) * 3u + (x >> 3);
      acc = (acc - 0x1e563408u) * 3u + (x >> 4);
      acc = (acc ^ 0x33020ccdu) * 3u + (x >> 5);
      acc = (acc ^ 0x15fa8b65u) * 3u + (x >> 6);
      acc = (acc ^ 0x047b2c10u) * 3u + (x >> 7);
      acc = (acc + 0x757f1cbau) * 3u + (x >> 8);
      acc = (acc ^ 0x730f37f1u) * 3u + (x >> 9);
      acc = (acc + 0x63087e52u) * 3u + (x >> 10);
      acc = (acc + 0x35f10300u) * 3u + (x >> 11);
      acc = (acc + 0x171e1a8cu) * 3u + (x >> 12);
      acc = (acc ^ 0x4305e986u) * 3u + (x >> 13);
      acc = (acc - 0x21f267e2u) * 3u + (x >> 14);
      acc = (acc + 0x4791c2e9u) * 3u + (x >> 15);
      acc = (acc + 0x5d7cfed1u) * 3u + (x >> 16);
      acc = (acc ^ 0x7f7595b5u) * 3u + (x >> 17);
      acc = (acc + 0x64e27602u) * 3u + (x >> 18);
      acc = (acc + 0x28b88073u) * 3u + (x >> 19);
      acc = (acc - 0x7ddfcbc9u) * 3u + (x >> 20);
      acc = (acc ^ 0x736506ecu) * 3u + (x >> 21);
      acc = (acc - 0x4d4ca9c7u) * 3u + (x >> 22);
      acc = (acc ^ 0x24056360u) * 3u + (x >> 23);
      acc = (acc ^ 0x580dc5abu) * 3u + (x >> 24);
      acc = (acc + 0x50ea7da7u) * 3u + (x >> 25);
      acc = (acc + 0x54d1ac6bu) * 3u + (x >> 26);
      acc = (acc ^ 0x53158ce4u) * 3u + (x >> 27);
      acc = (acc + 0x65f456aau) * 3u + (x >> 28);
      acc = (acc - 0x321c1744u) * 3u + (x >> 29);
      acc = (acc - 0x03003005u) * 3u + (x >> 30);
      acc = (acc ^ 0x4a327e2du) * 3u + (x >> 0);
      acc = (acc + 0x5f49f0fcu) * 3u + (x >> 1);
      acc = (acc ^ 0x64950dc2u) * 3u + (x >> 2);
      acc = (acc ^ 0x138efef9u) * 3u + (x >> 3);
      acc = (acc ^ 0x6d94dd6du) * 3u + (x >> 4);
      acc = (acc ^ 0x0c5b4c59u) * 3u + (x >> 5);
      acc = (acc + 0x1a09a840u) * 3u + (x >> 6);
      acc = (acc - 0x491e99f5u) * 3u + (x >> 7);
      acc = (acc + 0x261f40dfu) * 3u + (x >> 8);
      acc = (acc ^ 0x4406c053u) * 3u + (x >> 9);
    } else {
      acc = (acc + 0x50cb407au) * 3u + (x >> 10);
      acc = (acc ^ 0x5f93d180u) * 3u + (x >> 11);
      acc = (acc - 0x076d490au) * 3u + (x >> 12);
      acc = (acc - 0x66692158u) * 3u + (x >> 13);
      acc = (acc - 0x34145e87u) * 3u + (x >> 14);
      acc = (acc + 0x14a0b00bu) * 3u + (x >> 15);
      acc = (acc ^ 0x692fd360u) * 3u + (x >> 16);
      acc = (acc - 0x23797d45u) * 3u + (x >> 17);
      acc = (acc ^ 0x4944f2ceu) * 3u + (x >> 18);
      acc = (acc - 0x0c89c001u) * 3u + (x >> 19);
      acc = (acc + 0x2097798cu) * 3u + (x >> 20);
      acc = (acc ^ 0x78e10e70u) * 3u + (x >> 21);
      acc = (acc ^ 0x57fa49e5u) * 3u + (x >> 22);
      acc = (acc ^ 0x4c3ac6fcu) * 3u + (x >> 23);
      acc = (acc ^ 0x429a7079u) * 3u + (x >> 24);
      acc = (acc ^ 0x3d1926acu) * 3u + (x >> 25);
      acc = (acc - 0x7bb1d124u) * 3u + (x >> 26);
      acc = (acc + 0x64f54969u) * 3u + (x >> 27);
      acc = (acc - 0x2ad64ce9u) * 3u + (x >> 28);
      acc = (acc + 0x296259c8u) * 3u + (x >> 29);
      acc = (acc - 0x35372235u) * 3u + (x >> 30);
      acc = (acc - 0x7f405bc8u) * 3u + (x >> 0);
      acc = (acc ^ 0x3853933du) * 3u + (x >> 1);
      acc = (acc ^ 0x5534a034u) * 3u + (x >> 2);
      acc = (acc + 0x6d6b987au) * 3u + (x >> 3);
      acc = (acc + 0x31419775u) * 3u + (x >> 4);
      acc = (acc + 0x173910e3u) * 3u + (x >> 5);
      acc = (acc - 0x578a60d8u) * 3u + (x >> 6);
      acc = (acc ^ 0x1751f579u) * 3u + (x >> 7);
      acc = (acc ^ 0x3d376642u) * 3u + (x >> 8);
      acc = (acc - 0x4223b8aau) * 3u + (x >> 9);
      acc = (acc + 0x33bf9157u) * 3u + (x >> 10);
      acc = (acc ^ 0x69ac0f03u) * 3u + (x >> 11);
      acc = (acc - 0x69f44612u) * 3u + (x >> 12);
      acc = (acc ^ 0x35c2e229u) * 3u + (x >> 13);
      acc = (acc ^ 0x452e704du) * 3u + (x >> 14);
      acc = (acc ^ 0x0fe321ecu) * 3u + (x >> 15);
      acc = (acc - 0x470b4fadu) * 3u + (x >> 16);
      acc = (acc + 0x5c327a6du) * 3u + (x >> 17);
      acc = (acc + 0x37495c5eu) * 3u + (x >> 18);
      acc = (acc + 0x45619fc0u) * 3u + (x >> 19);
      acc = (acc ^ 0x627292f8u) * 3u + (x >> 20);
      acc = (acc ^ 0x7223c68au) * 3u + (x >> 21);
      acc = (acc + 0x4fe04802u) * 3u + (x >> 22);
      acc = (acc + 0x209342cau) * 3u + (x >> 23);
      acc = (acc - 0x6cd9e62au) * 3u + (x >> 24);
      acc = (acc - 0x79281c19u) * 3u + (x >> 25);
      acc = (acc + 0x7d652135u) * 3u + (x >> 26);
      acc = (acc ^ 0x12b92a01u) * 3u + (x >> 27);
      acc = (acc ^ 0x77d8c569u) * 3u + (x >> 28);
      acc = (acc + 0x3f9b6bb2u) * 3u + (x >> 29);
      acc = (acc + 0x394afbe9u) * 3u + (x >> 30);
      acc = (acc - 0x26edf1bdu) * 3u + (x >> 0);
      acc = (acc - 0x1be03df0u) * 3u + (x >> 1);
      acc = (acc + 0x75134107u) * 3u + (x >> 2);
      acc = (acc + 0x0a1fb43bu) * 3u + (x >> 3);
      acc = (acc + 0x202ab6fau) * 3u + (x >> 4);
      acc = (acc - 0x099f9c9fu) * 3u + (x >> 5);
      acc = (acc + 0x4dc4ac8cu) * 3u + (x >> 6);
      acc = (acc - 0x4075916eu) * 3u + (x >> 7);
      acc = (acc - 0x6ffb726au) * 3u + (x >> 8);
      acc = (acc + 0x1cb4ba55u) * 3u + (x >> 9);
      acc = (acc ^ 0x1202952fu) * 3u + (x >> 10);
      acc = (acc ^ 0x31135de9u) * 3u + (x >> 11);
      acc = (acc + 0x42c927b9u) * 3u + (x >> 12);
      acc = (acc + 0x004b7fd0u) * 3u + (x >> 13);
      acc = (acc ^ 0x4d307fe4u) * 3u + (x >> 14);
      acc = (acc ^ 0x47529194u) * 3u + (x >> 15);
      acc = (acc ^ 0x3e0b25cdu) * 3u + (x >> 16);
      acc = (acc - 0x3c19c315u) * 3u + (x >> 17);
      acc = (acc + 0x3f3f37eau) * 3u + (x >> 18);
      acc = (acc - 0x696c63d6u) * 3u + (x >> 19);
      acc = (acc + 0x4eb19fcau) * 3u + (x >> 20);
      acc = (acc + 0x0593dba2u) * 3u + (x >> 21);
      acc = (acc - 0x7f914286u) * 3u + (x >> 22);
      acc = (acc + 0x6b86290bu) * 3u + (x >> 23);
      acc = (acc + 0x41db898eu) * 3u + (x >> 24);
      acc = (acc ^ 0x6ca06496u) * 3u + (x >> 25);
      acc = (acc ^ 0x3a0ea6e1u) * 3u + (x >> 26);
      acc = (acc - 0x08ba9bd9u) * 3u + (x >> 27);
      acc = (acc - 0x568a8c29u) * 3u + (x >> 28);
      acc = (acc ^ 0x6ba99d01u) * 3u + (x >> 29);
      acc = (acc + 0x6577bb54u) * 3u + (x >> 30);
      acc = (acc ^ 0x01ba985au) * 3u + (x >> 0);
      acc = (acc + 0x114340ffu) * 3u + (x >> 1);
      acc = (acc + 0x7ee5e857u) * 3u + (x >> 2);
      acc = (acc + 0x4fcc9a5cu) * 3u + (x >> 3);
      acc = (acc ^ 0x3b164943u) * 3u + (x >> 4);
      acc = (acc ^ 0x38b079e1u) * 3u + (x >> 5);
      acc = (acc + 0x4b80b828u) * 3u + (x >> 6);
      acc = (acc - 0x7eea6fe1u) * 3u + (x >> 7);
      acc = (acc + 0x2ff3c23cu) * 3u + (x >> 8);
      acc = (acc ^ 0x7c2c6a87u) * 3u + (x >> 9);
      acc = (acc - 0x0e71597au) * 3u + (x >> 10);
      acc = (acc ^ 0x25795c18u) * 3u + (x >> 11);
      acc = (acc + 0x0dea6e4eu) * 3u + (x >> 12);
      acc = (acc - 0x060c8804u) * 3u + (x >> 13);
      acc = (acc ^ 0x245448c8u) * 3u + (x >> 14);
      acc = (acc - 0x0d456be0u) * 3u + (x >> 15);
      acc = (acc + 0x0f650638u) * 3u + (x >> 16);
      acc = (acc ^ 0x64b0bb14u) * 3u + (x >> 17);
      acc = (acc - 0x506f68acu) * 3u + (x >> 18);
      acc = (acc + 0x1cfb0a06u) * 3u + (x >> 19);
      acc = (acc ^ 0x2a66f913u) * 3u + (x >> 20);
      acc = (acc + 0x30d0a2b8u) * 3u + (x >> 21);
      acc = (acc + 0x77b5abcbu) * 3u + (x >> 22);
      acc = (acc - 0x4fd3e758u) * 3u + (x >> 23);
      acc = (acc ^ 0x60ed33a0u) * 3u + (x >> 24);
      acc = (acc ^ 0x54ea2061u) * 3u + (x >> 25);
      acc = (acc + 0x2b54af77u) * 3u + (x >> 26);
      acc = (acc + 0x00bc22cbu) * 3u + (x >> 27);
      acc = (acc + 0x47a164e4u) * 3u + (x >> 28);
      acc = (acc ^ 0x59f9bb79u) * 3u + (x >> 29);
      acc = (acc - 0x1fab5884u) * 3u + (x >> 30);
      acc = (acc ^ 0x35185376u) * 3u + (x >> 0);
      acc = (acc ^ 0x5b4c0d73u) * 3u + (x >> 1);
      acc = (acc + 0x6eb4fff8u) * 3u + (x >> 2);
      acc = (acc - 0x0c9c20efu) * 3u + (x >> 3);
      acc = (acc + 0x7934f0b8u) * 3u + (x >> 4);
      acc = (acc - 0x5f6a35d9u) * 3u + (x >> 5);
      acc = (acc + 0x7243d47cu) * 3u + (x >> 6);
      acc = (acc ^ 0x52c4641bu) * 3u + (x >> 7);
      acc = (acc + 0x797b1538u) * 3u + (x >> 8);
      acc = (acc + 0x692a4f0eu) * 3u + (x >> 9);
      acc = (acc + 0x679f2d9eu) * 3u + (x >> 10);
      acc = (acc + 0x602533dcu) * 3u + (x >> 11);
      acc = (acc + 0x76cc0573u) * 3u + (x >> 12);
      acc = (acc ^ 0x0fdf7cc6u) * 3u + (x >> 13);
      acc = (acc - 0x31e7aed1u) * 3u + (x >> 14);
      acc = (acc - 0x10170d2bu) * 3u + (x >> 15);
      acc = (acc ^ 0x56cd42d2u) * 3u + (x >> 16);
      acc = (acc ^ 0x45b669f7u) * 3u + (x >> 17);
      acc = (acc ^ 0x0b286c70u) * 3u + (x >> 18);
      acc = (acc ^ 0x5105122au) * 3u + (x >> 19);
      acc = (acc + 0x4c22cab7u) * 3u + (x >> 20);
      acc = (acc + 0x10b99ac9u) * 3u + (x >> 21);
      acc = (acc + 0x3bdea8c3u) * 3u + (x >> 22);
      acc = (acc - 0x79a5fd62u) * 3u + (x >> 23);
      acc = (acc ^ 0x773afe02u) * 3u + (x >> 24);
      acc = (acc ^ 0x40449aa0u) * 3u + (x >> 25);
      acc = (acc + 0x7e544d56u) * 3u + (x >> 26);
      acc = (acc + 0x7f1d490eu) * 3u + (x >> 27);
      acc = (acc - 0x023a80a2u) * 3u + (x >> 28);
      acc = (acc - 0x4da60990u) * 3u + (x >> 29);
      acc = (acc - 0x26bc9858u) * 3u + (x >> 30);
      acc = (acc ^ 0x3c73d5f4u) * 3u + (x >> 0);
      acc = (acc ^ 0x51cdf2f9u) * 3u + (x >> 1);
      acc = (acc - 0x5ca2c132u) * 3u + (x >> 2);
      acc = (acc - 0x143a5180u) * 3u + (x >> 3);
      acc = (acc ^ 0x32830689u) * 3u + (x >> 4);
      acc = (acc + 0x28f1a81bu) * 3u + (x >> 5);
      acc = (acc + 0x6862bf79u) * 3u + (x >> 6);
      acc = (acc ^ 0x08ab4ae4u) * 3u + (x >> 7);
      acc = (acc + 0x5364e64du) * 3u + (x >> 8);
      acc = (acc + 0x6d32a901u) * 3u + (x >> 9);
      acc = (acc ^ 0x1279688cu) * 3u + (x >> 10);
      acc = (acc + 0x15866ffbu) * 3u + (x >> 11);
      acc = (acc ^ 0x18af266cu) * 3u + (x >> 12);
      acc = (acc - 0x7f9c1321u) * 3u + (x >> 13);
      acc = (acc + 0x726c2c95u) * 3u + (x >> 14);
      acc = (acc + 0x3bf449fdu) * 3u + (x >> 15);
      acc = (acc ^ 0x6ab6114fu) * 3u + (x >> 16);
      acc = (acc - 0x3c2496ebu) * 3u + (x >> 17);
      acc = (acc ^ 0x1f04a6ffu) * 3u + (x >> 18);
      acc = (acc ^ 0x4b354e93u) * 3u + (x >> 19);
      acc = (acc ^ 0x4485c04fu) * 3u + (x >> 20);
      acc = (acc - 0x4109d8d6u) * 3u + (x >> 21);
      acc = (acc + 0x42a55162u) * 3u + (x >> 22);
      acc = (acc + 0x707c5f3du) * 3u + (x >> 23);
      acc = (acc + 0x2f8c6c08u) * 3u + (x >> 24);
      acc = (acc + 0x3c49fdbdu) * 3u + (x >> 25);
      acc = (acc - 0x4806d26fu) * 3u + (x >> 26);
      acc = (acc ^ 0x30312932u) * 3u + (x >> 27);
      acc = (acc ^ 0x10970046u) * 3u + (x >> 28);
      acc = (acc + 0x406c6132u) * 3u + (x >> 29);
      acc = (acc - 0x3b3bc813u) * 3u + (x >> 30);
      acc = (acc - 0x19bd2640u) * 3u + (x >> 0);
      acc = (acc + 0x76c32dcdu) * 3u + (x >> 1);
      acc = (acc + 0x1a327537u) * 3u + (x >> 2);
      acc = (acc + 0x798a0d59u) * 3u + (x >> 3);
      acc = (acc ^ 0x72c39a28u) * 3u + (x >> 4);
      acc = (acc ^ 0x0a5527a2u) * 3u + (x >> 5);
      acc = (acc + 0x3b9edacbu) * 3u + (x >> 6);
      acc = (acc + 0x0ce66f73u) * 3u + (x >> 7);
      acc = (acc + 0x31b4932cu) * 3u + (x >> 8);
      acc = (acc - 0x5f4aebebu) * 3u + (x >> 9);
      acc = (acc ^ 0x2d819d38u) * 3u + (x >> 10);
      acc = (acc - 0x428bf773u) * 3u + (x >> 11);
      acc = (acc + 0x019f7781u) * 3u + (x >> 12);
      acc = (acc + 0x5985ea3fu) * 3u + (x >> 13);
      acc = (acc ^ 0x09969e7cu) * 3u + (x >> 14);
      acc = (acc + 0x570b534du) * 3u + (x >> 15);
      acc = (acc + 0x0b4e7f7cu) * 3u + (x >> 16);
      acc = (acc + 0x414205c6u) * 3u + (x >> 17);
      acc = (acc + 0x3414c2dcu) * 3u + (x >> 18);
      acc = (acc ^ 0x53c69b0au) * 3u + (x >> 19);
      acc = (acc + 0x5f2ee40du) * 3u + (x >> 20);
      acc = (acc + 0x4fec0f40u) * 3u + (x >> 21);
      acc = (acc + 0x34128822u) * 3u + (x >> 22);
      acc = (acc - 0x7ee14b90u) * 3u + (x >> 23);
      acc = (acc + 0x7bc71df3u) * 3u + (x >> 24);
      acc = (acc + 0x687dd512u) * 3u + (x >> 25);
      acc = (acc - 0x65322a48u) * 3u + (x >> 26);
      acc = (acc - 0x2790bb01u) * 3u + (x >> 27);
      acc = (acc - 0x1755c6deu) * 3u + (x >> 28);
      acc = (acc ^ 0x29e78b06u) * 3u + (x >> 29);
      acc = (acc ^ 0x456b312cu) * 3u + (x >> 30);
      acc = (acc - 0x48866d48u) * 3u + (x >> 0);
      acc = (acc ^ 0x4ebe9880u) * 3u + (x >> 1);
      acc = (acc ^ 0x0d25f954u) * 3u + (x >> 2);
      acc = (acc ^ 0x5b7042dfu) * 3u + (x >> 3);
      acc = (acc + 0x6a9c2a33u) * 3u + (x >> 4);
      acc = (acc - 0x5d20c6a6u) * 3u + (x >> 5);
      acc = (acc ^ 0x327bcda3u) * 3u + (x >> 6);
      acc = (acc + 0x67ac56f8u) * 3u + (x >> 7);
      acc = (acc ^ 0x018120f8u) * 3u + (x >> 8);
      acc = (acc ^ 0x2814c437u) * 3u + (x >> 9);
      acc = (acc + 0x1d10e931u) * 3u + (x >> 10);
      acc = (acc - 0x67fde1c3u) * 3u + (x >> 11);
      acc = (acc ^ 0x5d5ec1adu) * 3u + (x >> 12);
      acc = (acc + 0x299c858du) * 3u + (x >> 13);
      acc = (acc + 0x03cc2f9bu) * 3u + (x >> 14);
      acc = (acc - 0x247aabb5u) * 3u + (x >> 15);
      acc = (acc + 0x658f62d1u) * 3u + (x >> 16);
      acc = (acc - 0x5eef9b8bu) * 3u + (x >> 17);
      acc = (acc + 0x2bf39775u) * 3u + (x >> 18);
      acc = (acc ^ 0x5912eb60u) * 3u + (x >> 19);
      acc = (acc - 0x296cb08cu) * 3u + (x >> 20);
      acc = (acc + 0x2bfa1f10u) * 3u + (x >> 21);
      acc = (acc ^ 0x1bd9d912u) * 3u + (x >> 22);
      acc = (acc + 0x7d920a56u) * 3u + (x >> 23);
      acc = (acc + 0x4d36a8edu) * 3u + (x >> 24);
      acc = (acc ^ 0x0b22a431u) * 3u + (x >> 25);
      acc = (acc + 0x5084c63fu) * 3u + (x >> 26);
      acc = (acc + 0x634d1952u) * 3u + (x >> 27);
      acc = (acc - 0x2907db86u) * 3u + (x >> 28);
      acc = (acc - 0x38d9e9abu) * 3u + (x >> 29);
      acc = (acc - 0x678c4cb9u) * 3u + (x >> 30);
      acc = (acc ^ 0x3234752bu) * 3u + (x >> 0);
      acc = (acc - 0x2ed6d460u) * 3u + (x >> 1);
      acc = (acc + 0x37d7d190u) * 3u + (x >> 2);
      acc = (acc - 0x6655b9f0u) * 3u + (x >> 3);
      acc = (acc ^ 0x280f005du) * 3u + (x >> 4);
      acc = (acc + 0x5bf508a0u) * 3u + (x >> 5);
      acc = (acc + 0x26437a8eu) * 3u + (x >> 6);
      acc = (acc + 0x314df386u) * 3u + (x >> 7);
      acc = (acc - 0x09c2cd73u) * 3u + (x >> 8);
      acc = (acc + 0x52fef478u) * 3u + (x >> 9);
      acc = (acc - 0x63cc537bu) * 3u + (x >> 10);
      acc = (acc - 0x74aaf340u) * 3u + (x >> 11);
      acc = (acc - 0x4e640cd4u) * 3u + (x >> 12);
      acc = (acc ^ 0x6b89d463u) * 3u + (x >> 13);
      acc = (acc ^ 0x3fcf6d85u) * 3u + (x >> 14);
      acc = (acc - 0x63a366aau) * 3u + (x >> 15);
      acc = (acc ^ 0x5e113423u) * 3u + (x >> 16);
      acc = (acc + 0x7037e034u) * 3u + (x >> 17);
      acc = (acc + 0x05fbec3au) * 3u + (x >> 18);
      acc = (acc ^ 0x7d4ffa0fu) * 3u + (x >> 19);
      acc = (acc ^ 0x3c39679du) * 3u + (x >> 20);
      acc = (acc + 0x75526e31u) * 3u + (x >> 21);
      acc = (acc ^ 0x7924dedeu) * 3u + (x >> 22);
      acc = (acc + 0x1b69567eu) * 3u + (x >> 23);
      acc = (acc ^ 0x20e27c17u) * 3u + (x >> 24);
      acc = (acc ^ 0x6e3bbc97u) * 3u + (x >> 25);
      acc = (acc ^ 0x177a8334u) * 3u + (x >> 26);
      acc = (acc + 0x0a6fb154u) * 3u + (x >> 27);
      acc = (acc + 0x2159702bu) * 3u + (x >> 28);
      acc = (acc - 0x50505652u) * 3u + (x >> 29);
      acc = (acc + 0x1478c7b9u) * 3u + (x >> 30);
      acc = (acc - 0x60bb9aeeu) * 3u + (x >> 0);
      acc = (acc + 0x22dd113cu) * 3u + (x >> 1);
      acc = (acc - 0x10fe52d4u) * 3u + (x >> 2);
      acc = (acc + 0x1c0df645u) * 3u + (x >> 3);
      acc = (acc ^ 0x21b1aed2u) * 3u + (x >> 4);
      acc = (acc + 0x49b29bbeu) * 3u + (x >> 5);
      acc = (acc + 0x389bc3dcu) * 3u + (x >> 6);
      acc = (acc - 0x59d4697fu) * 3u + (x >> 7);
      acc = (acc + 0x40918a58u) * 3u + (x >> 8);
      acc = (acc - 0x52e71cf8u) * 3u + (x >> 9);
      acc = (acc ^ 0x4665ea19u) * 3u + (x >> 10);
      acc = (acc ^ 0x24c1276cu) * 3u + (x >> 11);
      acc = (acc + 0x7ae85484u) * 3u + (x >> 12);
      acc = (acc - 0x434b4b94u) * 3u + (x >> 13);
      acc = (acc ^ 0x3cc63141u) * 3u + (x >> 14);
      acc = (acc + 0x5f4ce302u) * 3u + (x >> 15);
      acc = (acc + 0x32eddf6fu) * 3u + (x >> 16);
      acc = (acc + 0x67498314u) * 3u + (x >> 17);
      acc = (acc - 0x4737fed1u) * 3u + (x >> 18);
      acc = (acc ^ 0x53ec4b93u) * 3u + (x >> 19);
      acc = (acc ^ 0x2b32ada9u) * 3u + (x >> 20);
      acc = (acc - 0x1d75cc23u) * 3u + (x >> 21);
      acc = (acc - 0x0c6f2fccu) * 3u + (x >> 22);
      acc = (acc ^ 0x5c1a7c01u) * 3u + (x >> 23);
      acc = (acc ^ 0x1ac7a46cu) * 3u + (x >> 24);
      acc = (acc - 0x64edfce5u) * 3u + (x >> 25);
      acc = (acc ^ 0x5f186904u) * 3u + (x >> 26);
      acc = (acc ^ 0x60307b75u) * 3u + (x >> 27);
      acc = (acc ^ 0x256d1082u) * 3u + (x >> 28);
      acc = (acc + 0x54b13301u) * 3u + (x >> 29);
      acc = (acc + 0x71395e71u) * 3u + (x >> 30);
      acc = (acc - 0x2d3fe297u) * 3u + (x >> 0);
      acc = (acc ^ 0x0c5cd43bu) * 3u + (x >> 1);
      acc = (acc ^ 0x40ef5ec2u) * 3u + (x >> 2);
      acc = (acc - 0x5009c0a9u) * 3u + (x >> 3);
      acc = (acc - 0x00755f64u) * 3u + (x >> 4);
      acc = (acc + 0x08a6ab0fu) * 3u + (x >> 5);
      acc = (acc ^ 0x263cc4dcu) * 3u + (x >> 6);
      acc = (acc ^ 0x6ea6d05eu) * 3u + (x >> 7);
      acc = (acc + 0x5d359777u) * 3u + (x >> 8);
      acc = (acc ^ 0x21cc4751u) * 3u + (x >> 9);
      acc = (acc - 0x3a2db00au) * 3u + (x >> 10);
      acc = (acc + 0x0bab5f9fu) * 3u + (x >> 11);
      acc = (acc + 0x0decb3b5u) * 3u + (x >> 12);
      acc = (acc ^ 0x5aded3cau) * 3u + (x >> 13);
      acc = (acc - 0x1b3a953cu) * 3u + (x >> 14);
      acc = (acc - 0x5b6e48b0u) * 3u + (x >> 15);
      acc = (acc ^ 0x39690919u) * 3u + (x >> 16);
      acc = (acc - 0x4d187e3eu) * 3u + (x >> 17);
      acc = (acc + 0x223be9e7u) * 3u + (x >> 18);
      acc = (acc - 0x5dc18bceu) * 3u + (x >> 19);
      acc = (acc + 0x79932a50u) * 3u + (x >> 20);
      acc = (acc + 0x227ee409u) * 3u + (x >> 21);
      acc = (acc - 0x3e5bcce6u) * 3u + (x >> 22);
      acc = (acc ^ 0x263961d1u) * 3u + (x >> 23);
      acc = (acc + 0x1886a7bau) * 3u + (x >> 24);
      acc = (acc - 0x250a82a2u) * 3u + (x >> 25);
      acc = (acc ^ 0x450f002au) * 3u + (x >> 26);
      acc = (acc + 0x43a538c4u) * 3u + (x >> 27);
      acc = (acc - 0x0e5e928cu) * 3u + (x >> 28);
      acc = (acc - 0x59af6769u) * 3u + (x >> 29);
      acc = (acc - 0x7199e0b3u) * 3u + (x >> 30);
      acc = (acc + 0x7e2b86d1u) * 3u + (x >> 0);
      acc = (acc + 0x2a43f047u) * 3u + (x >> 1);
      acc = (acc + 0x0b43b6ddu) * 3u + (x >> 2);
      acc = (acc ^ 0x0675295fu) * 3u + (x >> 3);
      acc = (acc + 0x2f87466eu) * 3u + (x >> 4);
      acc = (acc + 0x28c26bb2u) * 3u + (x >> 5);
      acc = (acc + 0x1adbe533u) * 3u + (x >> 6);
      acc = (acc + 0x327f82f8u) * 3u + (x >> 7);
      acc = (acc + 0x69c60d1bu) * 3u + (x >> 8);
      acc = (acc - 0x6a4d76e6u) * 3u + (x >> 9);
      acc = (acc - 0x2cb52c32u) * 3u + (x >> 10);
      acc = (acc + 0x4f33b0eeu) * 3u + (x >> 11);
      acc = (acc - 0x4cde3e5au) * 3u + (x >> 12);
      acc = (acc - 0x0c69e424u) * 3u + (x >> 13);
      acc = (acc - 0x7a594f67u) * 3u + (x >> 14);
      acc = (acc ^ 0x01a01d42u) * 3u + (x >> 15);
      acc = (acc - 0x6fc820d2u) * 3u + (x >> 16);
      acc = (acc + 0x771ba4bau) * 3u + (x >> 17);
    }
    x = x * 1103515245u + 12345u;
  }
  return acc;
}

__attribute__((noinline)) int far_branches(int a) {
  int r = 0;
  if (a == 0) { sink = 0x73d63426u; r += 0; }
  else sink = 0x2ce678feu ^ (u32)a;
  if (a == 1) { sink = 0x39d7c140u; r += 1; }
  else sink = 0x1af3bda5u ^ (u32)a;
  if (a == 2) { sink = 0x42ecdcf9u; r += 2; }
  else sink = 0x3b77cbb4u ^ (u32)a;
  if (a == 3) { sink = 0x09eff2b4u; r += 3; }
  else sink = 0x1f8e6521u ^ (u32)a;
  if (a == 4) { sink = 0x55e4615bu; r += 4; }
  else sink = 0x43678856u ^ (u32)a;
  if (a == 5) { sink = 0x0d72cb97u; r += 5; }
  else sink = 0x4417c530u ^ (u32)a;
  if (a == 6) { sink = 0x6fa126a8u; r += 6; }
  else sink = 0x43ea7471u ^ (u32)a;
  if (a == 7) { sink = 0x4bad8e0eu; r += 7; }
  else sink = 0x378d04eau ^ (u32)a;
  if (a == 8) { sink = 0x15de2868u; r += 8; }
  else sink = 0x03e5f684u ^ (u32)a;
  if (a == 9) { sink = 0x2b7604feu; r += 9; }
  else sink = 0x42a78500u ^ (u32)a;
  if (a == 10) { sink = 0x3c71a896u; r += 10; }
  else sink = 0x33e92723u ^ (u32)a;
  if (a == 11) { sink = 0x28c06f25u; r += 11; }
  else sink = 0x53add817u ^ (u32)a;
  if (a == 12) { sink = 0x3122c815u; r += 12; }
  else sink = 0x63825046u ^ (u32)a;
  if (a == 13) { sink = 0x541c18d5u; r += 13; }
  else sink = 0x3d3a1902u ^ (u32)a;
  if (a == 14) { sink = 0x612390bau; r += 14; }
  else sink = 0x7830b083u ^ (u32)a;
  if (a == 15) { sink = 0x78de3361u; r += 15; }
  else sink = 0x01a23b4eu ^ (u32)a;
  if (a == 16) { sink = 0x06c9cd95u; r += 16; }
  else sink = 0x6fed41d7u ^ (u32)a;
  if (a == 17) { sink = 0x3bdc2efdu; r += 17; }
  else sink = 0x4ec8c223u ^ (u32)a;
  if (a == 18) { sink = 0x36436924u; r += 18; }
  else sink = 0x643d79f1u ^ (u32)a;
  if (a == 19) { sink = 0x13eadac3u; r += 19; }
  else sink = 0x2bea714du ^ (u32)a;
  if (a == 20) { sink = 0x25042c3du; r += 20; }
  else sink = 0x086d06d8u ^ (u32)a;
  if (a == 21) { sink = 0x06e315e3u; r += 21; }
  else sink = 0x1ca505c1u ^ (u32)a;
  if (a == 22) { sink = 0x1b4f463fu; r += 22; }
  else sink = 0x296c764du ^ (u32)a;
  if (a == 23) { sink = 0x5848fc64u; r += 23; }
  else sink = 0x244fbafcu ^ (u32)a;
  if (a == 24) { sink = 0x075b058bu; r += 24; }
  else sink = 0x07e7166bu ^ (u32)a;
  if (a == 25) { sink = 0x0aa989b4u; r += 25; }
  else sink = 0x236e536du ^ (u32)a;
  if (a == 26) { sink = 0x0aeade9bu; r += 26; }
  else sink = 0x115d27cfu ^ (u32)a;
  if (a == 27) { sink = 0x0bf3d0a7u; r += 27; }
  else sink = 0x10d5fe14u ^ (u32)a;
  if (a == 28) { sink = 0x5d082eeau; r += 28; }
  else sink = 0x33061fbcu ^ (u32)a;
  if (a == 29) { sink = 0x10e1fec9u; r += 29; }
  else sink = 0x62438362u ^ (u32)a;
  if (a == 30) { sink = 0x1b6bf273u; r += 30; }
  else sink = 0x3f1fb241u ^ (u32)a;
  if (a == 31) { sink = 0x34aa4a20u; r += 31; }
  else sink = 0x340252a6u ^ (u32)a;
  if (a == 32) { sink = 0x1caa0c48u; r += 32; }
  else sink = 0x08ab1715u ^ (u32)a;
  if (a == 33) { sink = 0x08d0323cu; r += 33; }
  else sink = 0x16646a40u ^ (u32)a;
  if (a == 34) { sink = 0x4990c224u; r += 34; }
  else sink = 0x7a243b32u ^ (u32)a;
  if (a == 35) { sink = 0x19918b8au; r += 35; }
  else sink = 0x21f59868u ^ (u32)a;
  if (a == 36) { sink = 0x190d78d3u; r += 36; }
  else sink = 0x347a7325u ^ (u32)a;
  if (a == 37) { sink = 0x4b61b0fdu; r += 37; }
  else sink = 0x51b315ecu ^ (u32)a;
  if (a == 38) { sink = 0x5625e671u; r += 38; }
  else sink = 0x6c7be37eu ^ (u32)a;
  if (a == 39) { sink = 0x42db5b4bu; r += 39; }
  else sink = 0x055ae98eu ^ (u32)a;
  if (a == 40) { sink = 0x59d4a28cu; r += 40; }
  else sink = 0x41b73d54u ^ (u32)a;
  if (a == 41) { sink = 0x4858079eu; r += 41; }
  else sink = 0x0c647801u ^ (u32)a;
  if (a == 42) { sink = 0x5e36d760u; r += 42; }
  else sink = 0x5221cbdau ^ (u32)a;
  if (a == 43) { sink = 0x79e08f86u; r += 43; }
  else sink = 0x49a35964u ^ (u32)a;
  if (a == 44) { sink = 0x07ee64feu; r += 44; }
  else sink = 0x69b52fc2u ^ (u32)a;
  if (a == 45) { sink = 0x07ffe38eu; r += 45; }
  else sink = 0x6fbb28f3u ^ (u32)a;
  if (a == 46) { sink = 0x192a2829u; r += 46; }
  else sink = 0x58c6aeeau ^ (u32)a;
  if (a == 47) { sink = 0x780c8fb0u; r += 47; }
  else sink = 0x0c5166f0u ^ (u32)a;
  if (a == 48) { sink = 0x3771690cu; r += 48; }
  else sink = 0x17448971u ^ (u32)a;
  if (a == 49) { sink = 0x49800525u; r += 49; }
  else sink = 0x2b9d7364u ^ (u32)a;
  if (a == 50) { sink = 0x6fa176acu; r += 50; }
  else sink = 0x00552293u ^ (u32)a;
  if (a == 51) { sink = 0x33b893a5u; r += 51; }
  else sink = 0x49d04ce5u ^ (u32)a;
  if (a == 52) { sink = 0x0dd09e51u; r += 52; }
  else sink = 0x011dd8b3u ^ (u32)a;
  if (a == 53) { sink = 0x5909a958u; r += 53; }
  else sink = 0x7da69370u ^ (u32)a;
  if (a == 54) { sink = 0x187f132du; r += 54; }
  else sink = 0x7dd1e6c7u ^ (u32)a;
  if (a == 55) { sink = 0x2f3ca661u; r += 55; }
  else sink = 0x7e9ce77au ^ (u32)a;
  if (a == 56) { sink = 0x58e1290du; r += 56; }
  else sink = 0x42b50c7cu ^ (u32)a;
  if (a == 57) { sink = 0x28ad5dc9u; r += 57; }
  else sink = 0x48a28354u ^ (u32)a;
  if (a == 58) { sink = 0x36f784ccu; r += 58; }
  else sink = 0x3b4563c7u ^ (u32)a;
  if (a == 59) { sink = 0x7f919c89u; r += 59; }
  else sink = 0x2a7147eau ^ (u32)a;
  if (a == 60) { sink = 0x1c23edeeu; r += 60; }
  else sink = 0x14b4b8d8u ^ (u32)a;
  if (a == 61) { sink = 0x7d83c1dfu; r += 61; }
  else sink = 0x1ac44e92u ^ (u32)a;
  if (a == 62) { sink = 0x539ef49cu; r += 62; }
  else sink = 0x5b09b845u ^ (u32)a;
  if (a == 63) { sink = 0x185ba663u; r += 63; }
  else sink = 0x66b9aaf9u ^ (u32)a;
  if (a == 64) { sink = 0x65047845u; r += 64; }
  else sink = 0x160f6d6eu ^ (u32)a;
  if (a == 65) { sink = 0x6c10b601u; r += 65; }
  else sink = 0x0671ce23u ^ (u32)a;
  if (a == 66) { sink = 0x5f381d79u; r += 66; }
  else sink = 0x34c411c3u ^ (u32)a;
  if (a == 67) { sink = 0x4d9aa696u; r += 67; }
  else sink = 0x4360c66au ^ (u32)a;
  if (a == 68) { sink = 0x6d956563u; r += 68; }
  else sink = 0x2bcd85d2u ^ (u32)a;
  if (a == 69) { sink = 0x611a245eu; r += 69; }
  else sink = 0x3bcb9bceu ^ (u32)a;
  if (a == 70) { sink = 0x75fe1142u; r += 70; }
  else sink = 0x207b3de0u ^ (u32)a;
  if (a == 71) { sink = 0x08aca106u; r += 71; }
  else sink = 0x59365783u ^ (u32)a;
  if (a == 72) { sink = 0x53a000dcu; r += 72; }
  else sink = 0x27c37e56u ^ (u32)a;
  if (a == 73) { sink = 0x73474aa9u; r += 73; }
  else sink = 0x52c602e2u ^ (u32)a;
  if (a == 74) { sink = 0x2b67a9fdu; r += 74; }
  else sink = 0x76917752u ^ (u32)a;
  if (a == 75) { sink = 0x7055114eu; r += 75; }
  else sink = 0x41d8b452u ^ (u32)a;
  if (a == 76) { sink = 0x3b246b47u; r += 76; }
  else sink = 0x20454643u ^ (u32)a;
  if (a == 77) { sink = 0x55848bffu; r += 77; }
  else sink = 0x7646cf57u ^ (u32)a;
  if (a == 78) { sink = 0x3ce9a9afu; r += 78; }
  else sink = 0x310afae0u ^ (u32)a;
  if (a == 79) { sink = 0x4479c074u; r += 79; }
  else sink = 0x4d2f9bbau ^ (u32)a;
  if (a == 80) { sink = 0x27937e85u; r += 80; }
  else sink = 0x27eeae0au ^ (u32)a;
  if (a == 81) { sink = 0x3f617877u; r += 81; }
  else sink = 0x53999ac8u ^ (u32)a;
  if (a == 82) { sink = 0x593ff3dfu; r += 82; }
  else sink = 0x293256b6u ^ (u32)a;
  if (a == 83) { sink = 0x3c787566u; r += 83; }
  else sink = 0x53fcba58u ^ (u32)a;
  if (a == 84) { sink = 0x307438e6u; r += 84; }
  else sink = 0x42396323u ^ (u32)a;
  if (a == 85) { sink = 0x1a0ffed5u; r += 85; }
  else sink = 0x2a23534au ^ (u32)a;
  if (a == 86) { sink = 0x1a04f280u; r += 86; }
  else sink = 0x3207d5a3u ^ (u32)a;
  if (a == 87) { sink = 0x625d165bu; r += 87; }
  else sink = 0x26a55215u ^ (u32)a;
  if (a == 88) { sink = 0x25f83e61u; r += 88; }
  else sink = 0x4d56c5aeu ^ (u32)a;
  if (a == 89) { sink = 0x4c22b1f4u; r += 89; }
  else sink = 0x6f571d36u ^ (u32)a;
  if (a == 90) { sink = 0x46191aa0u; r += 90; }
  else sink = 0x323991afu ^ (u32)a;
  if (a == 91) { sink = 0x1bf9b683u; r += 91; }
  else sink = 0x1b5bd042u ^ (u32)a;
  if (a == 92) { sink = 0x47e2cc36u; r += 92; }
  else sink = 0x34d982fbu ^ (u32)a;
  if (a == 93) { sink = 0x636a5479u; r += 93; }
  else sink = 0x76c338fau ^ (u32)a;
  if (a == 94) { sink = 0x08afbdedu; r += 94; }
  else sink = 0x033ae330u ^ (u32)a;
  if (a == 95) { sink = 0x66263f9fu; r += 95; }
  else sink = 0x6fc04d79u ^ (u32)a;
  if (a == 96) { sink = 0x38f2a031u; r += 96; }
  else sink = 0x4bd4a21cu ^ (u32)a;
  if (a == 97) { sink = 0x76997819u; r += 97; }
  else sink = 0x05a97aabu ^ (u32)a;
  if (a == 98) { sink = 0x244dd37fu; r += 98; }
  else sink = 0x41d8bf61u ^ (u32)a;
  if (a == 99) { sink = 0x679b4bbau; r += 99; }
  else sink = 0x01699af8u ^ (u32)a;
  if (a == 100) { sink = 0x3e06571bu; r += 100; }
  else sink = 0x6e1656d0u ^ (u32)a;
  if (a == 101) { sink = 0x6bd0cd12u; r += 101; }
  else sink = 0x3a8335f8u ^ (u32)a;
  if (a == 102) { sink = 0x3a85eed0u; r += 102; }
  else sink = 0x2e771bd6u ^ (u32)a;
  if (a == 103) { sink = 0x1fcc9634u; r += 103; }
  else sink = 0x7432f79du ^ (u32)a;
  if (a == 104) { sink = 0x6eba35e0u; r += 104; }
  else sink = 0x5021b420u ^ (u32)a;
  if (a == 105) { sink = 0x4282c843u; r += 105; }
  else sink = 0x190dcc94u ^ (u32)a;
  if (a == 106) { sink = 0x6b699f07u; r += 106; }
  else sink = 0x3e0dac1cu ^ (u32)a;
  if (a == 107) { sink = 0x666f0c32u; r += 107; }
  else sink = 0x280da853u ^ (u32)a;
  if (a == 108) { sink = 0x4003ff33u; r += 108; }
  else sink = 0x6c6fba96u ^ (u32)a;
  if (a == 109) { sink = 0x7b951593u; r += 109; }
  else sink = 0x7487a00cu ^ (u32)a;
  if (a == 110) { sink = 0x050842f5u; r += 110; }
  else sink = 0x68cacfe6u ^ (u32)a;
  if (a == 111) { sink = 0x2edd27f7u; r += 111; }
  else sink = 0x53fb51b9u ^ (u32)a;
  if (a == 112) { sink = 0x02b8c92au; r += 112; }
  else sink = 0x63826536u ^ (u32)a;
  if (a == 113) { sink = 0x7d662a32u; r += 113; }
  else sink = 0x1b3bb890u ^ (u32)a;
  if (a == 114) { sink = 0x09c3e7c0u; r += 114; }
  else sink = 0x40502845u ^ (u32)a;
  if (a == 115) { sink = 0x37c714cfu; r += 115; }
  else sink = 0x292cfb34u ^ (u32)a;
  if (a == 116) { sink = 0x3326d90fu; r += 116; }
  else sink = 0x59242043u ^ (u32)a;
  if (a == 117) { sink = 0x19e0d64au; r += 117; }
  else sink = 0x74efd764u ^ (u32)a;
  if (a == 118) { sink = 0x3479b1f0u; r += 118; }
  else sink = 0x79c9cdb6u ^ (u32)a;
  if (a == 119) { sink = 0x041f8d71u; r += 119; }
  else sink = 0x5eb2ad7eu ^ (u32)a;
  return r;
}

int main(void) {
  print_kx("big", big(12345, 37));
  for (int a = 0; a < 130; a += 7)
    print_kv("far", far_branches(a));
  return 0;
}
