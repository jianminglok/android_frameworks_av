/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "AudioResamplerSinc"
//#define LOG_NDEBUG 0

#include <string.h>
#include "AudioResamplerSinc.h"
#include <dlfcn.h>
#include <cutils/properties.h>
#include <stdlib.h>
#include <utils/Log.h>

namespace android {
// ----------------------------------------------------------------------------


/*
 * These coeficients are computed with the "fir" utility found in
 * tools/resampler_tools
 * cmd-line: fir -l 7 -s 48000 -c 20478
 */
const int32_t AudioResamplerSinc::mFirCoefsUp[] = {
        0x6d374bc7, 0x6d35278a, 0x6d2ebafe, 0x6d24069d, 0x6d150b35, 0x6d01c9e3, 0x6cea4418, 0x6cce7b97, 0x6cae7272, 0x6c8a2b0f, 0x6c61a823, 0x6c34ecb5, 0x6c03fc1c, 0x6bced9ff, 0x6b958a54, 0x6b581163, 0x6b1673c1, 0x6ad0b652, 0x6a86de48, 0x6a38f123, 0x69e6f4b1, 0x6990ef0b, 0x6936e697, 0x68d8e206, 0x6876e855, 0x681100c9, 0x67a732f4, 0x673986ac, 0x66c80413, 0x6652b392, 0x65d99dd5, 0x655ccbd3, 0x64dc46c3, 0x64581823, 0x63d049b4, 0x6344e578, 0x62b5f5b2, 0x622384e8, 0x618d9ddc, 0x60f44b91, 0x60579947, 0x5fb79278, 0x5f1442dc, 0x5e6db665, 0x5dc3f93c, 0x5d1717c4, 0x5c671e96, 0x5bb41a80, 0x5afe1886, 0x5a4525df, 0x59894ff3, 0x58caa45b, 0x580930e1, 0x5745037c, 0x567e2a51, 0x55b4b3af, 0x54e8ae13, 0x541a281e, 0x5349309e, 0x5275d684, 0x51a028e8, 0x50c83704, 0x4fee1037, 0x4f11c3fe, 0x4e3361f7, 0x4d52f9df, 0x4c709b8e, 0x4b8c56f8, 0x4aa63c2c, 0x49be5b50, 0x48d4c4a2, 0x47e98874, 0x46fcb72d, 0x460e6148, 0x451e9750, 0x442d69de, 0x433ae99c, 0x4247273f, 0x41523389, 0x405c1f43, 0x3f64fb40, 0x3e6cd85b, 0x3d73c772, 0x3c79d968, 0x3b7f1f23, 0x3a83a989, 0x3987897f, 0x388acfe9, 0x378d8da8, 0x368fd397, 0x3591b28b, 0x34933b50, 0x33947eab, 0x32958d55, 0x319677fa, 0x30974f3b, 0x2f9823a8, 0x2e9905c1, 0x2d9a05f4, 0x2c9b349e, 0x2b9ca203, 0x2a9e5e57, 0x29a079b2, 0x28a30416, 0x27a60d6a, 0x26a9a57b, 0x25addbf9, 0x24b2c075, 0x23b86263, 0x22bed116, 0x21c61bc0, 0x20ce516f, 0x1fd7810f, 0x1ee1b965, 0x1ded0911, 0x1cf97e8b, 0x1c072823, 0x1b1613ff, 0x1a26501b, 0x1937ea47, 0x184af025, 0x175f6f2b, 0x1675749e, 0x158d0d95, 0x14a646f6, 0x13c12d73, 0x12ddcd8f, 0x11fc3395,
        0x111c6ba0, 0x103e8192, 0x0f62811a, 0x0e8875ad, 0x0db06a89, 0x0cda6ab5, 0x0c0680fe, 0x0b34b7f5, 0x0a6519f4, 0x0997b116, 0x08cc873c, 0x0803a60a, 0x073d16e7, 0x0678e2fc, 0x05b71332, 0x04f7b037, 0x043ac276, 0x0380521c, 0x02c86715, 0x0213090c, 0x01603f6e, 0x00b01162, 0x000285d0, 0xff57a35e, 0xfeaf706f, 0xfe09f323, 0xfd673159, 0xfcc730aa, 0xfc29f670, 0xfb8f87bd, 0xfaf7e963, 0xfa631fef, 0xf9d12fab, 0xf9421c9d, 0xf8b5ea87, 0xf82c9ce7, 0xf7a636fa, 0xf722bbb5, 0xf6a22dcf, 0xf6248fb6, 0xf5a9e398, 0xf5322b61, 0xf4bd68b6, 0xf44b9cfe, 0xf3dcc959, 0xf370eea9, 0xf3080d8c, 0xf2a2265e, 0xf23f393b, 0xf1df45fd, 0xf1824c3e, 0xf1284b58, 0xf0d14267, 0xf07d3043, 0xf02c138a, 0xefddea9a, 0xef92b393, 0xef4a6c58, 0xef051290, 0xeec2a3a3, 0xee831cc3, 0xee467ae1, 0xee0cbab9, 0xedd5d8ca, 0xeda1d15c, 0xed70a07d, 0xed424205, 0xed16b196, 0xecedea99, 0xecc7e845, 0xeca4a59b, 0xec841d68, 0xec664a48, 0xec4b26a2, 0xec32acb0, 0xec1cd677, 0xec099dcf, 0xebf8fc64, 0xebeaebaf, 0xebdf6500, 0xebd6617b, 0xebcfda19, 0xebcbc7a7, 0xebca22cc, 0xebcae405, 0xebce03aa, 0xebd379eb, 0xebdb3ed5, 0xebe54a4f, 0xebf1941f, 0xec0013e8, 0xec10c12c, 0xec23934f, 0xec388194, 0xec4f8322, 0xec688f02, 0xec839c22, 0xeca0a156, 0xecbf9558, 0xece06ecb, 0xed032439, 0xed27ac16, 0xed4dfcc2, 0xed760c88, 0xed9fd1a2, 0xedcb4237, 0xedf8545b, 0xee26fe17, 0xee573562, 0xee88f026, 0xeebc2444, 0xeef0c78d, 0xef26cfca, 0xef5e32bd, 0xef96e61c, 0xefd0df9a, 0xf00c14e1, 0xf0487b98, 0xf0860962, 0xf0c4b3e0, 0xf10470b0, 0xf1453571, 0xf186f7c0, 0xf1c9ad40, 0xf20d4b92, 0xf251c85d, 0xf297194d, 0xf2dd3411,
        0xf3240e61, 0xf36b9dfd, 0xf3b3d8ac, 0xf3fcb43e, 0xf4462690, 0xf4902587, 0xf4daa718, 0xf525a143, 0xf5710a17, 0xf5bcd7b1, 0xf609003f, 0xf6557a00, 0xf6a23b44, 0xf6ef3a6e, 0xf73c6df4, 0xf789cc61, 0xf7d74c53, 0xf824e480, 0xf8728bb3, 0xf8c038d0, 0xf90de2d1, 0xf95b80cb, 0xf9a909ea, 0xf9f67577, 0xfa43bad2, 0xfa90d17b, 0xfaddb10c, 0xfb2a513b, 0xfb76a9dd, 0xfbc2b2e4, 0xfc0e6461, 0xfc59b685, 0xfca4a19f, 0xfcef1e20, 0xfd392498, 0xfd82adba, 0xfdcbb25a, 0xfe142b6e, 0xfe5c120f, 0xfea35f79, 0xfeea0d0c, 0xff30144a, 0xff756edc, 0xffba168d, 0xfffe054e, 0x00413536, 0x0083a081, 0x00c54190, 0x010612eb, 0x01460f41, 0x01853165, 0x01c37452, 0x0200d32c, 0x023d493c, 0x0278d1f2, 0x02b368e6, 0x02ed09d7, 0x0325b0ad, 0x035d5977, 0x0394006a, 0x03c9a1e5, 0x03fe3a6f, 0x0431c6b5, 0x0464438c, 0x0495adf2, 0x04c6030d, 0x04f54029, 0x052362ba, 0x0550685d, 0x057c4ed4, 0x05a7140b, 0x05d0b612, 0x05f93324, 0x0620899e, 0x0646b808, 0x066bbd0d, 0x068f9781, 0x06b2465b, 0x06d3c8bb, 0x06f41de3, 0x0713453d, 0x07313e56, 0x074e08e0, 0x0769a4b2, 0x078411c7, 0x079d503b, 0x07b56051, 0x07cc426c, 0x07e1f712, 0x07f67eec, 0x0809dac3, 0x081c0b84, 0x082d1239, 0x083cf010, 0x084ba654, 0x08593671, 0x0865a1f1, 0x0870ea7e, 0x087b11de, 0x088419f6, 0x088c04c8, 0x0892d470, 0x08988b2a, 0x089d2b4a, 0x08a0b740, 0x08a33196, 0x08a49cf0, 0x08a4fc0d, 0x08a451c0, 0x08a2a0f8, 0x089fecbb, 0x089c3824, 0x08978666, 0x0891dac8, 0x088b38a9, 0x0883a378, 0x087b1ebc, 0x0871ae0d, 0x08675516, 0x085c1794, 0x084ff957, 0x0842fe3d, 0x08352a35, 0x0826813e, 0x08170767, 0x0806c0cb, 0x07f5b193, 0x07e3ddf7,
        0x07d14a38, 0x07bdfaa5, 0x07a9f399, 0x07953976, 0x077fd0ac, 0x0769bdaf, 0x07530501, 0x073bab28, 0x0723b4b4, 0x070b2639, 0x06f20453, 0x06d853a2, 0x06be18cd, 0x06a3587e, 0x06881761, 0x066c5a27, 0x06502583, 0x06337e2a, 0x061668d2, 0x05f8ea30, 0x05db06fc, 0x05bcc3ed, 0x059e25b5, 0x057f310a, 0x055fea9d, 0x0540571a, 0x05207b2f, 0x05005b82, 0x04dffcb6, 0x04bf6369, 0x049e9433, 0x047d93a8, 0x045c6654, 0x043b10bd, 0x04199760, 0x03f7feb4, 0x03d64b27, 0x03b4811d, 0x0392a4f4, 0x0370bafc, 0x034ec77f, 0x032ccebb, 0x030ad4e1, 0x02e8de19, 0x02c6ee7f, 0x02a50a22, 0x02833506, 0x02617321, 0x023fc85c, 0x021e3891, 0x01fcc78f, 0x01db7914, 0x01ba50d2, 0x0199526b, 0x01788170, 0x0157e166, 0x013775bf, 0x011741df, 0x00f7491a, 0x00d78eb3, 0x00b815da, 0x0098e1b3, 0x0079f54c, 0x005b53a4, 0x003cffa9, 0x001efc35, 0x00014c12, 0xffe3f1f7, 0xffc6f08a, 0xffaa4a5d, 0xff8e01f1, 0xff7219b3, 0xff5693fe, 0xff3b731b, 0xff20b93e, 0xff066889, 0xfeec830d, 0xfed30ac5, 0xfeba0199, 0xfea16960, 0xfe8943dc, 0xfe7192bd, 0xfe5a579d, 0xfe439407, 0xfe2d496f, 0xfe177937, 0xfe0224b0, 0xfded4d13, 0xfdd8f38b, 0xfdc5192d, 0xfdb1befc, 0xfd9ee5e7, 0xfd8c8ecc, 0xfd7aba74, 0xfd696998, 0xfd589cdc, 0xfd4854d3, 0xfd3891fd, 0xfd2954c8, 0xfd1a9d91, 0xfd0c6ca2, 0xfcfec233, 0xfcf19e6b, 0xfce50161, 0xfcd8eb17, 0xfccd5b82, 0xfcc25285, 0xfcb7cff0, 0xfcadd386, 0xfca45cf7, 0xfc9b6be5, 0xfc92ffe1, 0xfc8b186d, 0xfc83b4fc, 0xfc7cd4f0, 0xfc76779e, 0xfc709c4d, 0xfc6b4233, 0xfc66687a, 0xfc620e3d, 0xfc5e328c, 0xfc5ad465, 0xfc57f2be, 0xfc558c7c, 0xfc53a07b, 0xfc522d88, 0xfc513266, 0xfc50adcc,
        0xfc509e64, 0xfc5102d0, 0xfc51d9a6, 0xfc53216f, 0xfc54d8ae, 0xfc56fdda, 0xfc598f60, 0xfc5c8ba5, 0xfc5ff105, 0xfc63bdd3, 0xfc67f05a, 0xfc6c86dd, 0xfc717f97, 0xfc76d8bc, 0xfc7c9079, 0xfc82a4f4, 0xfc89144d, 0xfc8fdc9f, 0xfc96fbfc, 0xfc9e7074, 0xfca63810, 0xfcae50d6, 0xfcb6b8c4, 0xfcbf6dd8, 0xfcc86e09, 0xfcd1b74c, 0xfcdb4793, 0xfce51ccb, 0xfcef34e1, 0xfcf98dbe, 0xfd04254a, 0xfd0ef969, 0xfd1a0801, 0xfd254ef4, 0xfd30cc24, 0xfd3c7d73, 0xfd4860c2, 0xfd5473f3, 0xfd60b4e7, 0xfd6d2180, 0xfd79b7a1, 0xfd86752e, 0xfd93580d, 0xfda05e23, 0xfdad855b, 0xfdbacb9e, 0xfdc82edb, 0xfdd5ad01, 0xfde34403, 0xfdf0f1d6, 0xfdfeb475, 0xfe0c89db, 0xfe1a7009, 0xfe286505, 0xfe3666d5, 0xfe447389, 0xfe528931, 0xfe60a5e5, 0xfe6ec7c0, 0xfe7cece2, 0xfe8b1373, 0xfe99399f, 0xfea75d97, 0xfeb57d92, 0xfec397cf, 0xfed1aa92, 0xfedfb425, 0xfeedb2da, 0xfefba508, 0xff09890f, 0xff175d53, 0xff252042, 0xff32d04f, 0xff406bf8, 0xff4df1be, 0xff5b602c, 0xff68b5d5, 0xff75f153, 0xff831148, 0xff90145e, 0xff9cf947, 0xffa9bebe, 0xffb66386, 0xffc2e669, 0xffcf463a, 0xffdb81d6, 0xffe79820, 0xfff38806, 0xffff507b, 0x000af07f, 0x00166718, 0x0021b355, 0x002cd44d, 0x0037c922, 0x004290fc, 0x004d2b0e, 0x00579691, 0x0061d2ca, 0x006bdf05, 0x0075ba95, 0x007f64da, 0x0088dd38, 0x0092231e, 0x009b3605, 0x00a4156b, 0x00acc0da, 0x00b537e1, 0x00bd7a1c, 0x00c5872a, 0x00cd5eb7, 0x00d50075, 0x00dc6c1e, 0x00e3a175, 0x00eaa045, 0x00f16861, 0x00f7f9a3, 0x00fe53ef, 0x0104772e, 0x010a6353, 0x01101858, 0x0115963d, 0x011add0b, 0x011fecd3, 0x0124c5ab, 0x012967b1, 0x012dd30a, 0x013207e4, 0x01360670,
        0x0139cee9, 0x013d618d, 0x0140bea5, 0x0143e67c, 0x0146d965, 0x014997bb, 0x014c21db, 0x014e782a, 0x01509b14, 0x01528b08, 0x0154487b, 0x0155d3e8, 0x01572dcf, 0x015856b6, 0x01594f25, 0x015a17ab, 0x015ab0db, 0x015b1b4e, 0x015b579e, 0x015b666c, 0x015b485b, 0x015afe14, 0x015a8843, 0x0159e796, 0x01591cc0, 0x01582878, 0x01570b77, 0x0155c678, 0x01545a3c, 0x0152c783, 0x01510f13, 0x014f31b2, 0x014d3029, 0x014b0b45, 0x0148c3d2, 0x01465a9f, 0x0143d07f, 0x01412643, 0x013e5cc0, 0x013b74ca, 0x01386f3a, 0x01354ce7, 0x01320ea9, 0x012eb55a, 0x012b41d3, 0x0127b4f1, 0x01240f8e, 0x01205285, 0x011c7eb2, 0x011894f0, 0x0114961b, 0x0110830f, 0x010c5ca6, 0x010823ba, 0x0103d927, 0x00ff7dc4, 0x00fb126b, 0x00f697f3, 0x00f20f32, 0x00ed78ff, 0x00e8d62d, 0x00e4278f, 0x00df6df7, 0x00daaa34, 0x00d5dd16, 0x00d10769, 0x00cc29f7, 0x00c7458a, 0x00c25ae8, 0x00bd6ad7, 0x00b87619, 0x00b37d70, 0x00ae8198, 0x00a9834e, 0x00a4834c, 0x009f8249, 0x009a80f8, 0x0095800c, 0x00908034, 0x008b821b, 0x0086866b, 0x00818dcb, 0x007c98de, 0x0077a845, 0x0072bc9d, 0x006dd680, 0x0068f687, 0x00641d44, 0x005f4b4a, 0x005a8125, 0x0055bf60, 0x00510682, 0x004c570f, 0x0047b186, 0x00431666, 0x003e8628, 0x003a0141, 0x00358824, 0x00311b41, 0x002cbb03, 0x002867d2, 0x00242213, 0x001fea27, 0x001bc06b, 0x0017a53b, 0x001398ec, 0x000f9bd2, 0x000bae3c, 0x0007d075, 0x000402c8, 0x00004579, 0xfffc98c9, 0xfff8fcf7, 0xfff5723d, 0xfff1f8d2, 0xffee90eb, 0xffeb3ab8, 0xffe7f666, 0xffe4c41e, 0xffe1a408, 0xffde9646, 0xffdb9af8, 0xffd8b23b, 0xffd5dc28, 0xffd318d6, 0xffd06858, 0xffcdcabe, 0xffcb4014,
        0xffc8c866, 0xffc663b9, 0xffc41212, 0xffc1d373, 0xffbfa7d9, 0xffbd8f40, 0xffbb89a1, 0xffb996f3, 0xffb7b728, 0xffb5ea31, 0xffb42ffc, 0xffb28876, 0xffb0f388, 0xffaf7118, 0xffae010b, 0xffaca344, 0xffab57a1, 0xffaa1e02, 0xffa8f641, 0xffa7e039, 0xffa6dbc0, 0xffa5e8ad, 0xffa506d2, 0xffa43603, 0xffa3760e, 0xffa2c6c2, 0xffa227ec, 0xffa19957, 0xffa11acb, 0xffa0ac11, 0xffa04cf0, 0xff9ffd2c, 0xff9fbc89, 0xff9f8ac9, 0xff9f67ae, 0xff9f52f7, 0xff9f4c65, 0xff9f53b4, 0xff9f68a1, 0xff9f8ae9, 0xff9fba47, 0xff9ff674, 0xffa03f2b, 0xffa09425, 0xffa0f519, 0xffa161bf, 0xffa1d9cf, 0xffa25cfe, 0xffa2eb04, 0xffa38395, 0xffa42668, 0xffa4d332, 0xffa589a6, 0xffa6497c, 0xffa71266, 0xffa7e41a, 0xffa8be4c, 0xffa9a0b1, 0xffaa8afe, 0xffab7ce7, 0xffac7621, 0xffad7662, 0xffae7d5f, 0xffaf8acd, 0xffb09e63, 0xffb1b7d8, 0xffb2d6e1, 0xffb3fb37, 0xffb52490, 0xffb652a7, 0xffb78533, 0xffb8bbed, 0xffb9f691, 0xffbb34d8, 0xffbc767f, 0xffbdbb42, 0xffbf02dd, 0xffc04d0f, 0xffc19996, 0xffc2e832, 0xffc438a3, 0xffc58aaa, 0xffc6de09, 0xffc83285, 0xffc987e0, 0xffcadde1, 0xffcc344c, 0xffcd8aeb, 0xffcee183, 0xffd037e0, 0xffd18dcc, 0xffd2e311, 0xffd4377d, 0xffd58ade, 0xffd6dd02, 0xffd82dba, 0xffd97cd6, 0xffdaca2a, 0xffdc1588, 0xffdd5ec6, 0xffdea5bb, 0xffdfea3c, 0xffe12c22, 0xffe26b48, 0xffe3a788, 0xffe4e0bf, 0xffe616c8, 0xffe74984, 0xffe878d3, 0xffe9a494, 0xffeaccaa, 0xffebf0fa, 0xffed1166, 0xffee2dd7, 0xffef4632, 0xfff05a60, 0xfff16a4a, 0xfff275db, 0xfff37d00, 0xfff47fa5, 0xfff57db8, 0xfff67729, 0xfff76be9, 0xfff85be8, 0xfff9471b, 0xfffa2d74, 0xfffb0ee9, 0xfffbeb70,
        0xfffcc300, 0xfffd9592, 0xfffe631e, 0xffff2b9f, 0xffffef10, 0x0000ad6e, 0x000166b6, 0x00021ae5, 0x0002c9fd, 0x000373fb, 0x000418e2, 0x0004b8b3, 0x00055371, 0x0005e921, 0x000679c5, 0x00070564, 0x00078c04, 0x00080dab, 0x00088a62, 0x00090230, 0x0009751e, 0x0009e337, 0x000a4c85, 0x000ab112, 0x000b10ec, 0x000b6c1d, 0x000bc2b3, 0x000c14bb, 0x000c6244, 0x000cab5c, 0x000cf012, 0x000d3075, 0x000d6c97, 0x000da486, 0x000dd854, 0x000e0812, 0x000e33d3, 0x000e5ba7, 0x000e7fa1, 0x000e9fd5, 0x000ebc54, 0x000ed533, 0x000eea84, 0x000efc5c, 0x000f0ace, 0x000f15ef, 0x000f1dd2, 0x000f228d, 0x000f2434, 0x000f22dc, 0x000f1e99, 0x000f1781, 0x000f0da8, 0x000f0125, 0x000ef20b, 0x000ee070, 0x000ecc69, 0x000eb60b, 0x000e9d6b, 0x000e829e, 0x000e65ba, 0x000e46d3, 0x000e25fd, 0x000e034f, 0x000ddedb, 0x000db8b7, 0x000d90f6, 0x000d67ae, 0x000d3cf1, 0x000d10d5, 0x000ce36b, 0x000cb4c8, 0x000c84ff, 0x000c5422, 0x000c2245, 0x000bef79, 0x000bbbd2, 0x000b8760, 0x000b5235, 0x000b1c64, 0x000ae5fc, 0x000aaf0f, 0x000a77ac, 0x000a3fe5, 0x000a07c9, 0x0009cf67, 0x000996ce, 0x00095e0e, 0x00092535, 0x0008ec50, 0x0008b36e, 0x00087a9c, 0x000841e8, 0x0008095d, 0x0007d108, 0x000798f5, 0x00076130, 0x000729c4, 0x0006f2bb, 0x0006bc21, 0x000685ff, 0x0006505f, 0x00061b4b, 0x0005e6cb, 0x0005b2e8, 0x00057faa, 0x00054d1a, 0x00051b3e, 0x0004ea1d, 0x0004b9c0, 0x00048a2b, 0x00045b65, 0x00042d74, 0x0004005e, 0x0003d426, 0x0003a8d2, 0x00037e65, 0x000354e5, 0x00032c54, 0x000304b7, 0x0002de0e, 0x0002b85f, 0x000293aa, 0x00026ff2, 0x00024d39, 0x00022b7f, 0x00020ac7, 0x0001eb10,
        0x00000000 // this one is needed for lerping the last coefficient
};

/*
 * These coefficients are optimized for 48KHz -> 44.1KHz
 * cmd-line: fir -l 7 -s 48000 -c 16600
 */
const int32_t AudioResamplerSinc::mFirCoefsDown[] = {
        0x58888889, 0x58875d88, 0x5883dc96, 0x587e05e0, 0x5875d9b3, 0x586b587d, 0x585e82c6, 0x584f593a, 0x583ddc9f, 0x582a0dde, 0x5813edfb, 0x57fb7e1a, 0x57e0bf7f, 0x57c3b389, 0x57a45bb8, 0x5782b9aa, 0x575ecf1a, 0x57389de0, 0x571027f6, 0x56e56f6f, 0x56b8767e, 0x56893f73, 0x5657ccbb, 0x562420e2, 0x55ee3e8d, 0x55b62882, 0x557be1a0, 0x553f6ce6, 0x5500cd6d, 0x54c0066a, 0x547d1b2e, 0x54380f26, 0x53f0e5da, 0x53a7a2ed, 0x535c4a1e, 0x530edf46, 0x52bf6657, 0x526de360, 0x521a5a86, 0x51c4d00c, 0x516d484a, 0x5113c7b6, 0x50b852d9, 0x505aee59, 0x4ffb9ef2, 0x4f9a6979, 0x4f3752d9, 0x4ed26016, 0x4e6b9649, 0x4e02faa3, 0x4d98926b, 0x4d2c62fd, 0x4cbe71cc, 0x4c4ec45e, 0x4bdd6050, 0x4b6a4b53, 0x4af58b2b, 0x4a7f25b0, 0x4a0720cd, 0x498d8283, 0x491250e1, 0x4895920c, 0x48174c37, 0x479785ab, 0x471644bd, 0x46938fd7, 0x460f6d70, 0x4589e411, 0x4502fa51, 0x447ab6d5, 0x43f12053, 0x43663d8d, 0x42da1554, 0x424cae85, 0x41be100a, 0x412e40db, 0x409d47f9, 0x400b2c72, 0x3f77f561, 0x3ee3a9e7, 0x3e4e5132, 0x3db7f27a, 0x3d2094ff, 0x3c88400b, 0x3beefaee, 0x3b54cd01, 0x3ab9bda6, 0x3a1dd444, 0x39811848, 0x38e39127, 0x3845465a, 0x37a63f5f, 0x370683ba, 0x36661af1, 0x35c50c90, 0x35236024, 0x34811d3f, 0x33de4b72, 0x333af253, 0x32971979, 0x31f2c87a, 0x314e06ed, 0x30a8dc6a, 0x30035089, 0x2f5d6ade, 0x2eb732fe, 0x2e10b07d, 0x2d69eaeb, 0x2cc2e9d4, 0x2c1bb4c4, 0x2b745340, 0x2acccccc, 0x2a2528e6, 0x297d6f06, 0x28d5a6a0, 0x282dd722, 0x278607f2, 0x26de4072, 0x263687fa, 0x258ee5dd, 0x24e76163, 0x244001cf, 0x2398ce58, 0x22f1ce2e, 0x224b0876, 0x21a4844b, 0x20fe48be, 0x20585cd5,
        0x1fb2c78a, 0x1f0d8fcb, 0x1e68bc7d, 0x1dc45475, 0x1d205e7d, 0x1c7ce150, 0x1bd9e39e, 0x1b376c06, 0x1a95811c, 0x19f42964, 0x19536b51, 0x18b34d4a, 0x1813d5a3, 0x17750aa3, 0x16d6f27f, 0x1639935b, 0x159cf34b, 0x15011851, 0x1466085d, 0x13cbc94f, 0x133260f3, 0x1299d502, 0x12022b24, 0x116b68ed, 0x10d593dd, 0x1040b162, 0x0facc6d4, 0x0f19d979, 0x0e87ee81, 0x0df70b09, 0x0d673417, 0x0cd86e9d, 0x0c4abf78, 0x0bbe2b70, 0x0b32b735, 0x0aa86763, 0x0a1f407f, 0x099746f9, 0x09107f29, 0x088aed4f, 0x08069598, 0x07837c17, 0x0701a4c8, 0x06811392, 0x0601cc40, 0x0583d28b, 0x05072a0f, 0x048bd653, 0x0411dac7, 0x03993abf, 0x0321f97b, 0x02ac1a20, 0x02379fbb, 0x01c48d42, 0x0152e590, 0x00e2ab69, 0x0073e179, 0x00068a52, 0xff9aa86c, 0xff303e29, 0xfec74dd1, 0xfe5fd993, 0xfdf9e383, 0xfd956da0, 0xfd3279cd, 0xfcd109d6, 0xfc711f6d, 0xfc12bc2a, 0xfbb5e18f, 0xfb5a9103, 0xfb00cbd4, 0xfaa89339, 0xfa51e84e, 0xf9fccc18, 0xf9a93f82, 0xf9574361, 0xf906d86d, 0xf8b7ff4b, 0xf86ab883, 0xf81f0487, 0xf7d4e3b0, 0xf78c5641, 0xf7455c62, 0xf6fff625, 0xf6bc2385, 0xf679e463, 0xf639388a, 0xf5fa1fae, 0xf5bc996b, 0xf580a547, 0xf54642b1, 0xf50d70ff, 0xf4d62f74, 0xf4a07d3b, 0xf46c5967, 0xf439c2f9, 0xf408b8d8, 0xf3d939d9, 0xf3ab44b9, 0xf37ed821, 0xf353f2a5, 0xf32a92c3, 0xf302b6e6, 0xf2dc5d64, 0xf2b7847f, 0xf2942a64, 0xf2724d2e, 0xf251eae4, 0xf2330179, 0xf2158ece, 0xf1f990b1, 0xf1df04de, 0xf1c5e8ff, 0xf1ae3aaa, 0xf197f765, 0xf1831ca6, 0xf16fa7d0, 0xf15d9634, 0xf14ce516, 0xf13d91a7, 0xf12f9909, 0xf122f84e, 0xf117ac79, 0xf10db27d, 0xf1050741, 0xf0fda799, 0xf0f7904e, 0xf0f2be1a,
        0xf0ef2dab, 0xf0ecdba0, 0xf0ebc48a, 0xf0ebe4f1, 0xf0ed394e, 0xf0efbe0d, 0xf0f36f92, 0xf0f84a32, 0xf0fe4a39, 0xf1056be8, 0xf10dab74, 0xf117050a, 0xf12174cd, 0xf12cf6d5, 0xf1398732, 0xf14721ec, 0xf155c300, 0xf1656666, 0xf176080d, 0xf187a3db, 0xf19a35b1, 0xf1adb969, 0xf1c22ad4, 0xf1d785c1, 0xf1edc5f5, 0xf204e733, 0xf21ce537, 0xf235bbb8, 0xf24f6669, 0xf269e0fa, 0xf2852715, 0xf2a13462, 0xf2be0485, 0xf2db9321, 0xf2f9dbd3, 0xf318da38, 0xf33889ec, 0xf358e688, 0xf379eba4, 0xf39b94d7, 0xf3bdddb7, 0xf3e0c1db, 0xf4043cd8, 0xf4284a45, 0xf44ce5ba, 0xf4720ace, 0xf497b51a, 0xf4bde03a, 0xf4e487c9, 0xf50ba766, 0xf5333ab3, 0xf55b3d52, 0xf583aaec, 0xf5ac7f29, 0xf5d5b5b7, 0xf5ff4a47, 0xf6293890, 0xf6537c4a, 0xf67e1134, 0xf6a8f311, 0xf6d41dab, 0xf6ff8cce, 0xf72b3c4f, 0xf7572808, 0xf7834bd7, 0xf7afa3a3, 0xf7dc2b58, 0xf808deec, 0xf835ba59, 0xf862b9a0, 0xf88fd8cc, 0xf8bd13f0, 0xf8ea6724, 0xf917ce8a, 0xf945464f, 0xf972caa4, 0xf9a057c6, 0xf9cde9fb, 0xf9fb7d90, 0xfa290edf, 0xfa569a49, 0xfa841c3a, 0xfab19127, 0xfadef591, 0xfb0c4601, 0xfb397f0d, 0xfb669d55, 0xfb939d83, 0xfbc07c4c, 0xfbed3671, 0xfc19c8bf, 0xfc46300d, 0xfc72693e, 0xfc9e7141, 0xfcca4511, 0xfcf5e1b4, 0xfd21443e, 0xfd4c69cd, 0xfd774f8e, 0xfda1f2b7, 0xfdcc508d, 0xfdf66662, 0xfe203193, 0xfe49af8a, 0xfe72ddbf, 0xfe9bb9b7, 0xfec44103, 0xfeec7141, 0xff14481d, 0xff3bc351, 0xff62e0a2, 0xff899de5, 0xffaff8f9, 0xffd5efce, 0xfffb8060, 0x0020a8b7, 0x004566eb, 0x0069b920, 0x008d9d89, 0x00b11264, 0x00d415ff, 0x00f6a6b5, 0x0118c2ef, 0x013a6922, 0x015b97d1, 0x017c4d8f, 0x019c88f9, 0x01bc48bd,
        0x01db8b94, 0x01fa5045, 0x021895a6, 0x02365a98, 0x02539e0b, 0x02705efd, 0x028c9c77, 0x02a85592, 0x02c38972, 0x02de3749, 0x02f85e57, 0x0311fde7, 0x032b1552, 0x0343a3ff, 0x035ba961, 0x037324f6, 0x038a164c, 0x03a07cfa, 0x03b658a7, 0x03cba904, 0x03e06dcf, 0x03f4a6d1, 0x040853e2, 0x041b74e4, 0x042e09c4, 0x0440127d, 0x04518f14, 0x04627f9b, 0x0472e42e, 0x0482bcf5, 0x04920a24, 0x04a0cbf7, 0x04af02ba, 0x04bcaebe, 0x04c9d064, 0x04d66814, 0x04e27642, 0x04edfb6c, 0x04f8f819, 0x05036cdc, 0x050d5a51, 0x0516c11c, 0x051fa1ee, 0x0527fd7e, 0x052fd48d, 0x053727e8, 0x053df861, 0x054446d5, 0x054a1429, 0x054f614a, 0x05542f2f, 0x05587ed5, 0x055c5141, 0x055fa783, 0x056282ae, 0x0564e3e1, 0x0566cc3e, 0x05683cf1, 0x0569372c, 0x0569bc29, 0x0569cd27, 0x05696b6b, 0x05689842, 0x056754fe, 0x0565a2f9, 0x0563838f, 0x0560f824, 0x055e0222, 0x055aa2f6, 0x0556dc14, 0x0552aef5, 0x054e1d14, 0x054927f4, 0x0543d11a, 0x053e1a11, 0x05380465, 0x053191aa, 0x052ac373, 0x05239b5b, 0x051c1afe, 0x051443fa, 0x050c17f3, 0x0503988d, 0x04fac770, 0x04f1a647, 0x04e836bd, 0x04de7a82, 0x04d47346, 0x04ca22bc, 0x04bf8a97, 0x04b4ac8c, 0x04a98a54, 0x049e25a4, 0x04928037, 0x04869bc6, 0x047a7a0b, 0x046e1cc1, 0x046185a3, 0x0454b66c, 0x0447b0d7, 0x043a76a1, 0x042d0983, 0x041f6b39, 0x04119d7b, 0x0403a204, 0x03f57a8c, 0x03e728c9, 0x03d8ae73, 0x03ca0d3e, 0x03bb46dd, 0x03ac5d03, 0x039d5160, 0x038e25a2, 0x037edb76, 0x036f7486, 0x035ff27a, 0x035056f9, 0x0340a3a5, 0x0330da20, 0x0320fc08, 0x03110af8, 0x03010889, 0x02f0f64f, 0x02e0d5df, 0x02d0a8c6, 0x02c07090, 0x02b02ec6, 0x029fe4ec,
        0x028f9484, 0x027f3f0b, 0x026ee5fa, 0x025e8ac8, 0x024e2ee5, 0x023dd3c0, 0x022d7ac1, 0x021d254d, 0x020cd4c6, 0x01fc8a88, 0x01ec47ea, 0x01dc0e40, 0x01cbded8, 0x01bbbafd, 0x01aba3f2, 0x019b9afa, 0x018ba14e, 0x017bb826, 0x016be0b3, 0x015c1c20, 0x014c6b97, 0x013cd038, 0x012d4b20, 0x011ddd67, 0x010e8820, 0x00ff4c57, 0x00f02b13, 0x00e12558, 0x00d23c22, 0x00c37068, 0x00b4c31c, 0x00a6352a, 0x0097c778, 0x00897ae9, 0x007b5057, 0x006d4899, 0x005f647f, 0x0051a4d3, 0x00440a5a, 0x003695d5, 0x002947fc, 0x001c2183, 0x000f231a, 0x00024d68, 0xfff5a111, 0xffe91eb2, 0xffdcc6e4, 0xffd09a37, 0xffc49939, 0xffb8c471, 0xffad1c5f, 0xffa1a180, 0xff965449, 0xff8b352a, 0xff804490, 0xff7582e0, 0xff6af079, 0xff608db6, 0xff565aec, 0xff4c586c, 0xff42867e, 0xff38e569, 0xff2f756c, 0xff2636c2, 0xff1d29a0, 0xff144e36, 0xff0ba4ae, 0xff032d30, 0xfefae7db, 0xfef2d4cc, 0xfeeaf419, 0xfee345d5, 0xfedbca0b, 0xfed480c6, 0xfecd6a07, 0xfec685cf, 0xfebfd416, 0xfeb954d4, 0xfeb307f8, 0xfeaced6f, 0xfea70522, 0xfea14ef4, 0xfe9bcac5, 0xfe96786f, 0xfe9157cb, 0xfe8c68ab, 0xfe87aadd, 0xfe831e2e, 0xfe7ec263, 0xfe7a9741, 0xfe769c85, 0xfe72d1ed, 0xfe6f3731, 0xfe6bcc04, 0xfe689017, 0xfe658319, 0xfe62a4b3, 0xfe5ff48d, 0xfe5d7249, 0xfe5b1d89, 0xfe58f5ea, 0xfe56fb06, 0xfe552c76, 0xfe5389cc, 0xfe52129d, 0xfe50c676, 0xfe4fa4e5, 0xfe4ead73, 0xfe4ddfa8, 0xfe4d3b09, 0xfe4cbf19, 0xfe4c6b59, 0xfe4c3f47, 0xfe4c3a5e, 0xfe4c5c1b, 0xfe4ca3f4, 0xfe4d1160, 0xfe4da3d4, 0xfe4e5ac3, 0xfe4f359e, 0xfe5033d5, 0xfe5154d6, 0xfe52980d, 0xfe53fce6, 0xfe5582cb, 0xfe572926, 0xfe58ef5d, 0xfe5ad4d7,
        0xfe5cd8fa, 0xfe5efb2b, 0xfe613ace, 0xfe639746, 0xfe660ff5, 0xfe68a43c, 0xfe6b537e, 0xfe6e1d1b, 0xfe710072, 0xfe73fce5, 0xfe7711d2, 0xfe7a3e98, 0xfe7d8297, 0xfe80dd2e, 0xfe844dbc, 0xfe87d39f, 0xfe8b6e37, 0xfe8f1ce3, 0xfe92df02, 0xfe96b3f4, 0xfe9a9b19, 0xfe9e93d1, 0xfea29d7d, 0xfea6b77d, 0xfeaae135, 0xfeaf1a05, 0xfeb36152, 0xfeb7b67e, 0xfebc18ef, 0xfec0880a, 0xfec50334, 0xfec989d5, 0xfece1b54, 0xfed2b71b, 0xfed75c94, 0xfedc0b2a, 0xfee0c249, 0xfee5815e, 0xfeea47d8, 0xfeef1528, 0xfef3e8be, 0xfef8c20c, 0xfefda088, 0xff0283a5, 0xff076adc, 0xff0c55a4, 0xff114377, 0xff1633d0, 0xff1b262d, 0xff201a0c, 0xff250eee, 0xff2a0453, 0xff2ef9c1, 0xff33eebc, 0xff38e2cb, 0xff3dd578, 0xff42c64c, 0xff47b4d6, 0xff4ca0a2, 0xff518941, 0xff566e47, 0xff5b4f45, 0xff602bd4, 0xff65038a, 0xff69d601, 0xff6ea2d6, 0xff7369a7, 0xff782a12, 0xff7ce3bb, 0xff819645, 0xff864157, 0xff8ae498, 0xff8f7fb2, 0xff941251, 0xff989c25, 0xff9d1cdc, 0xffa1942a, 0xffa601c3, 0xffaa655e, 0xffaebeb2, 0xffb30d7c, 0xffb75177, 0xffbb8a62, 0xffbfb7ff, 0xffc3da11, 0xffc7f05c, 0xffcbfaa8, 0xffcff8be, 0xffd3ea6a, 0xffd7cf79, 0xffdba7b9, 0xffdf72fe, 0xffe33119, 0xffe6e1e1, 0xffea852e, 0xffee1ad8, 0xfff1a2bb, 0xfff51cb5, 0xfff888a4, 0xfffbe66b, 0xffff35ed, 0x0002770f, 0x0005a9b8, 0x0008cdd0, 0x000be344, 0x000ee9ff, 0x0011e1f0, 0x0014cb08, 0x0017a538, 0x001a7075, 0x001d2cb3, 0x001fd9eb, 0x00227816, 0x0025072f, 0x00278731, 0x0029f81b, 0x002c59ed, 0x002eaca8, 0x0030f04f, 0x003324e6, 0x00354a74, 0x003760ff, 0x00396892, 0x003b6135, 0x003d4af6, 0x003f25e1, 0x0040f206, 0x0042af73,
        0x00445e3a, 0x0045fe6e, 0x00479023, 0x0049136d, 0x004a8864, 0x004bef1e, 0x004d47b5, 0x004e9242, 0x004fcedf, 0x0050fdaa, 0x00521ebe, 0x0053323b, 0x0054383e, 0x005530e9, 0x00561c5b, 0x0056fab7, 0x0057cc20, 0x005890b9, 0x005948a7, 0x0059f40e, 0x005a9315, 0x005b25e2, 0x005bac9d, 0x005c276d, 0x005c967d, 0x005cf9f4, 0x005d51fd, 0x005d9ec3, 0x005de071, 0x005e1731, 0x005e4331, 0x005e649d, 0x005e7ba1, 0x005e886c, 0x005e8b2b, 0x005e840c, 0x005e733e, 0x005e58ef, 0x005e354e, 0x005e088c, 0x005dd2d6, 0x005d945e, 0x005d4d53, 0x005cfde5, 0x005ca645, 0x005c46a2, 0x005bdf2d, 0x005b7017, 0x005af990, 0x005a7bc9, 0x0059f6f2, 0x00596b3b, 0x0058d8d6, 0x00583ff2, 0x0057a0c0, 0x0056fb70, 0x00565032, 0x00559f36, 0x0054e8ac, 0x00542cc2, 0x00536baa, 0x0052a591, 0x0051daa6, 0x00510b19, 0x00503717, 0x004f5ece, 0x004e826d, 0x004da220, 0x004cbe15, 0x004bd678, 0x004aeb75, 0x0049fd39, 0x00490bef, 0x004817c2, 0x004720dd, 0x0046276a, 0x00452b92, 0x00442d80, 0x00432d5b, 0x00422b4c, 0x0041277c, 0x00402210, 0x003f1b31, 0x003e1304, 0x003d09b0, 0x003bff58, 0x003af423, 0x0039e833, 0x0038dbad, 0x0037ceb3, 0x0036c168, 0x0035b3ed, 0x0034a664, 0x003398ed, 0x00328ba7, 0x00317eb3, 0x0030722e, 0x002f6638, 0x002e5aec, 0x002d5069, 0x002c46c9, 0x002b3e2a, 0x002a36a5, 0x00293054, 0x00282b52, 0x002727b7, 0x0026259c, 0x00252518, 0x00242641, 0x00232930, 0x00222df8, 0x002134b0, 0x00203d6b, 0x001f483d, 0x001e5539, 0x001d6473, 0x001c75fb, 0x001b89e3, 0x001aa03b, 0x0019b913, 0x0018d47b, 0x0017f281, 0x00171334, 0x001636a0, 0x00155cd2, 0x001485d7, 0x0013b1ba, 0x0012e086,
        0x00121246, 0x00114703, 0x00107ec6, 0x000fb999, 0x000ef783, 0x000e388c, 0x000d7cba, 0x000cc414, 0x000c0ea0, 0x000b5c64, 0x000aad63, 0x000a01a2, 0x00095925, 0x0008b3f0, 0x00081204, 0x00077364, 0x0006d811, 0x0006400e, 0x0005ab5a, 0x000519f6, 0x00048be2, 0x0004011d, 0x000379a7, 0x0002f57d, 0x0002749e, 0x0001f708, 0x00017cb7, 0x000105a9, 0x000091da, 0x00002147, 0xffffb3eb, 0xffff49c1, 0xfffee2c6, 0xfffe7ef2, 0xfffe1e41, 0xfffdc0ad, 0xfffd6630, 0xfffd0ec3, 0xfffcba5f, 0xfffc68fd, 0xfffc1a97, 0xfffbcf23, 0xfffb869a, 0xfffb40f4, 0xfffafe29, 0xfffabe30, 0xfffa8100, 0xfffa4690, 0xfffa0ed7, 0xfff9d9cc, 0xfff9a764, 0xfff97796, 0xfff94a58, 0xfff91fa0, 0xfff8f764, 0xfff8d199, 0xfff8ae34, 0xfff88d2b, 0xfff86e74, 0xfff85203, 0xfff837cd, 0xfff81fc7, 0xfff809e6, 0xfff7f61f, 0xfff7e467, 0xfff7d4b1, 0xfff7c6f4, 0xfff7bb22, 0xfff7b132, 0xfff7a917, 0xfff7a2c6, 0xfff79e33, 0xfff79b52, 0xfff79a19, 0xfff79a7b, 0xfff79c6e, 0xfff79fe5, 0xfff7a4d5, 0xfff7ab33, 0xfff7b2f3, 0xfff7bc0a, 0xfff7c66d, 0xfff7d210, 0xfff7dee8, 0xfff7eceb, 0xfff7fc0c, 0xfff80c41, 0xfff81d80, 0xfff82fbc, 0xfff842ed, 0xfff85707, 0xfff86bff, 0xfff881cb, 0xfff89861, 0xfff8afb7, 0xfff8c7c3, 0xfff8e07b, 0xfff8f9d4, 0xfff913c6, 0xfff92e46, 0xfff9494c, 0xfff964ce, 0xfff980c3, 0xfff99d23, 0xfff9b9e3, 0xfff9d6fc, 0xfff9f465, 0xfffa1216, 0xfffa3006, 0xfffa4e2d, 0xfffa6c84, 0xfffa8b03, 0xfffaa9a3, 0xfffac85b, 0xfffae725, 0xfffb05f9, 0xfffb24d2, 0xfffb43a7, 0xfffb6273, 0xfffb812f, 0xfffb9fd5, 0xfffbbe5f, 0xfffbdcc6, 0xfffbfb07, 0xfffc191a, 0xfffc36fa, 0xfffc54a4, 0xfffc7210,
        0x00000000 // this one is needed for lerping the last coefficient
};

// we use 15 bits to interpolate between these samples
// this cannot change because the mul below rely on it.
static const int pLerpBits = 15;

static pthread_once_t once_control = PTHREAD_ONCE_INIT;
static readCoefficientsFn readResampleCoefficients = NULL;

/*static*/ AudioResamplerSinc::Constants AudioResamplerSinc::highQualityConstants;
/*static*/ AudioResamplerSinc::Constants AudioResamplerSinc::veryHighQualityConstants;

void AudioResamplerSinc::init_routine()
{
    // for high quality resampler, the parameters for coefficients are compile-time constants
    Constants *c = &highQualityConstants;
    c->coefsBits = RESAMPLE_FIR_LERP_INT_BITS;
    c->cShift = kNumPhaseBits - c->coefsBits;
    c->cMask = ((1<< c->coefsBits)-1) << c->cShift;
    c->pShift = kNumPhaseBits - c->coefsBits - pLerpBits;
    c->pMask = ((1<< pLerpBits)-1) << c->pShift;
    c->halfNumCoefs = RESAMPLE_FIR_NUM_COEF;

    // for very high quality resampler, the parameters are load-time constants
    veryHighQualityConstants = highQualityConstants;

    // Open the dll to get the coefficients for VERY_HIGH_QUALITY
    void *resampleCoeffLib = dlopen("libaudio-resampler.so", RTLD_NOW);
    ALOGV("Open libaudio-resampler library = %p", resampleCoeffLib);
    if (resampleCoeffLib == NULL) {
        ALOGE("Could not open audio-resampler library: %s", dlerror());
        return;
    }

    readResampleCoefficients = (readCoefficientsFn) dlsym(resampleCoeffLib,
            "readResamplerCoefficients");
    readResampleFirNumCoeffFn readResampleFirNumCoeff = (readResampleFirNumCoeffFn)
            dlsym(resampleCoeffLib, "readResampleFirNumCoeff");
    readResampleFirLerpIntBitsFn readResampleFirLerpIntBits = (readResampleFirLerpIntBitsFn)
            dlsym(resampleCoeffLib, "readResampleFirLerpIntBits");
    if (!readResampleCoefficients || !readResampleFirNumCoeff || !readResampleFirLerpIntBits) {
        readResampleCoefficients = NULL;
        dlclose(resampleCoeffLib);
        resampleCoeffLib = NULL;
        ALOGE("Could not find symbol: %s", dlerror());
        return;
    }

    c = &veryHighQualityConstants;
    // we have 16 coefs samples per zero-crossing
    c->coefsBits = readResampleFirLerpIntBits();
    ALOGV("coefsBits = %d", c->coefsBits);
    c->cShift = kNumPhaseBits - c->coefsBits;
    c->cMask = ((1<<c->coefsBits)-1) << c->cShift;
    c->pShift = kNumPhaseBits - c->coefsBits - pLerpBits;
    c->pMask = ((1<<pLerpBits)-1) << c->pShift;
    // number of zero-crossing on each side
    c->halfNumCoefs = readResampleFirNumCoeff();
    ALOGV("halfNumCoefs = %d", c->halfNumCoefs);
    // note that we "leak" resampleCoeffLib until the process exits
}

// ----------------------------------------------------------------------------

static inline
int32_t mulRL(int left, int32_t in, uint32_t vRL)
{
#if defined(__arm__) && !defined(__thumb__)
    int32_t out;
    if (left) {
        asm( "smultb %[out], %[in], %[vRL] \n"
             : [out]"=r"(out)
             : [in]"%r"(in), [vRL]"r"(vRL)
             : );
    } else {
        asm( "smultt %[out], %[in], %[vRL] \n"
             : [out]"=r"(out)
             : [in]"%r"(in), [vRL]"r"(vRL)
             : );
    }
    return out;
#else
    int16_t v = left ? int16_t(vRL) : int16_t(vRL>>16);
    return int32_t((int64_t(in) * v) >> 16);
#endif
}

static inline
int32_t mulAdd(int16_t in, int32_t v, int32_t a)
{
#if defined(__arm__) && !defined(__thumb__)
    int32_t out;
    asm( "smlawb %[out], %[v], %[in], %[a] \n"
         : [out]"=r"(out)
         : [in]"%r"(in), [v]"r"(v), [a]"r"(a)
         : );
    return out;
#else
    return a + int32_t((int64_t(v) * in) >> 16);
#endif
}

static inline
int32_t mulAddRL(int left, uint32_t inRL, int32_t v, int32_t a)
{
#if defined(__arm__) && !defined(__thumb__)
    int32_t out;
    if (left) {
        asm( "smlawb %[out], %[v], %[inRL], %[a] \n"
             : [out]"=r"(out)
             : [inRL]"%r"(inRL), [v]"r"(v), [a]"r"(a)
             : );
    } else {
        asm( "smlawt %[out], %[v], %[inRL], %[a] \n"
             : [out]"=r"(out)
             : [inRL]"%r"(inRL), [v]"r"(v), [a]"r"(a)
             : );
    }
    return out;
#else
    int16_t s = left ? int16_t(inRL) : int16_t(inRL>>16);
    return a + int32_t((int64_t(v) * s) >> 16);
#endif
}

// ----------------------------------------------------------------------------

AudioResamplerSinc::AudioResamplerSinc(int bitDepth,
        int inChannelCount, int32_t sampleRate, src_quality quality)
    : AudioResampler(bitDepth, inChannelCount, sampleRate, quality),
    mState(0)
{
    /*
     * Layout of the state buffer for 32 tap:
     *
     * "present" sample            beginning of 2nd buffer
     *                 v                v
     *  0              01               2              23              3
     *  0              F0               0              F0              F
     * [pppppppppppppppInnnnnnnnnnnnnnnnpppppppppppppppInnnnnnnnnnnnnnnn]
     *                 ^               ^ head
     *
     * p = past samples, convoluted with the (p)ositive side of sinc()
     * n = future samples, convoluted with the (n)egative side of sinc()
     * r = extra space for implementing the ring buffer
     *
     */

    // Load the constants for coefficients
    int ok = pthread_once(&once_control, init_routine);
    if (ok != 0) {
        ALOGE("%s pthread_once failed: %d", __func__, ok);
    }
    mConstants = (quality == VERY_HIGH_QUALITY) ? &veryHighQualityConstants : &highQualityConstants;
}


AudioResamplerSinc::~AudioResamplerSinc()
{
    delete[] mState;
}

void AudioResamplerSinc::init() {
    const Constants *c = mConstants;

    const size_t numCoefs = 2*c->halfNumCoefs;
    const size_t stateSize = numCoefs * mChannelCount * 2;
    mState = new int16_t[stateSize];
    memset(mState, 0, sizeof(int16_t)*stateSize);
    mImpulse = mState + (c->halfNumCoefs-1)*mChannelCount;
    mRingFull = mImpulse + (numCoefs+1)*mChannelCount;
}

void AudioResamplerSinc::resample(int32_t* out, size_t outFrameCount,
            AudioBufferProvider* provider)
{

    // FIXME store current state (up or down sample) and only load the coefs when the state
    // changes. Or load two pointers one for up and one for down in the init function.
    // Not critical now since the read functions are fast, but would be important if read was slow.
    if (mConstants == &veryHighQualityConstants && readResampleCoefficients) {
        ALOGV("get coefficient from libmm-audio resampler library");
        mFirCoefs = (mInSampleRate <= mSampleRate) ? readResampleCoefficients(true) :
                readResampleCoefficients(false);
    } else {
        ALOGV("Use default coefficients");
        mFirCoefs = (mInSampleRate <= mSampleRate) ? mFirCoefsUp : mFirCoefsDown;
    }

    // select the appropriate resampler
    switch (mChannelCount) {
    case 1:
        resample<1>(out, outFrameCount, provider);
        break;
    case 2:
        resample<2>(out, outFrameCount, provider);
        break;
    }

}


template<int CHANNELS>
void AudioResamplerSinc::resample(int32_t* out, size_t outFrameCount,
        AudioBufferProvider* provider)
{
    const Constants *c = mConstants;
    int16_t* impulse = mImpulse;
    uint32_t vRL = mVolumeRL;
    size_t inputIndex = mInputIndex;
    uint32_t phaseFraction = mPhaseFraction;
    uint32_t phaseIncrement = mPhaseIncrement;
    size_t outputIndex = 0;
    size_t outputSampleCount = outFrameCount * 2;
    size_t inFrameCount = (outFrameCount*mInSampleRate)/mSampleRate;

    while (outputIndex < outputSampleCount) {
        // buffer is empty, fetch a new one
        while (mBuffer.frameCount == 0) {
            mBuffer.frameCount = inFrameCount;
            provider->getNextBuffer(&mBuffer,
                                    calculateOutputPTS(outputIndex / 2));
            if (mBuffer.raw == NULL) {
                goto resample_exit;
            }
            const uint32_t phaseIndex = phaseFraction >> kNumPhaseBits;
            if (phaseIndex == 1) {
                // read one frame
                read<CHANNELS>(impulse, phaseFraction, mBuffer.i16, inputIndex);
            } else if (phaseIndex == 2) {
                // read 2 frames
                read<CHANNELS>(impulse, phaseFraction, mBuffer.i16, inputIndex);
                inputIndex++;
                if (inputIndex >= mBuffer.frameCount) {
                    inputIndex -= mBuffer.frameCount;
                    provider->releaseBuffer(&mBuffer);
                } else {
                    read<CHANNELS>(impulse, phaseFraction, mBuffer.i16, inputIndex);
                }
            }
        }
        int16_t *in = mBuffer.i16;
        const size_t frameCount = mBuffer.frameCount;

        // Always read-in the first samples from the input buffer
        int16_t* head = impulse + c->halfNumCoefs*CHANNELS;
        head[0] = in[inputIndex*CHANNELS + 0];
        if (CHANNELS == 2)
            head[1] = in[inputIndex*CHANNELS + 1];

        // handle boundary case
        int32_t l, r;
        while (outputIndex < outputSampleCount) {
            filterCoefficient<CHANNELS>(l, r, phaseFraction, impulse);
            out[outputIndex++] += 2 * mulRL(1, l, vRL);
            out[outputIndex++] += 2 * mulRL(0, r, vRL);

            phaseFraction += phaseIncrement;
            const uint32_t phaseIndex = phaseFraction >> kNumPhaseBits;
            if (phaseIndex == 1) {
                inputIndex++;
                if (inputIndex >= frameCount)
                    break;  // need a new buffer
                read<CHANNELS>(impulse, phaseFraction, in, inputIndex);
            } else if (phaseIndex == 2) {    // maximum value
                inputIndex++;
                if (inputIndex >= frameCount)
                    break;  // 0 frame available, 2 frames needed
                // read first frame
                read<CHANNELS>(impulse, phaseFraction, in, inputIndex);
                inputIndex++;
                if (inputIndex >= frameCount)
                    break;  // 0 frame available, 1 frame needed
                // read second frame
                read<CHANNELS>(impulse, phaseFraction, in, inputIndex);
            }
        }

        // if done with buffer, save samples
        if (inputIndex >= frameCount) {
            inputIndex -= frameCount;
            provider->releaseBuffer(&mBuffer);
        }
    }

resample_exit:
    mImpulse = impulse;
    mInputIndex = inputIndex;
    mPhaseFraction = phaseFraction;
}

template<int CHANNELS>
/***
* read()
*
* This function reads only one frame from input buffer and writes it in
* state buffer
*
**/
void AudioResamplerSinc::read(
        int16_t*& impulse, uint32_t& phaseFraction,
        const int16_t* in, size_t inputIndex)
{
    const Constants *c = mConstants;
    const uint32_t phaseIndex = phaseFraction >> kNumPhaseBits;
    impulse += CHANNELS;
    phaseFraction -= 1LU<<kNumPhaseBits;
    if (impulse >= mRingFull) {
        const size_t stateSize = (c->halfNumCoefs*2)*CHANNELS;
        memcpy(mState, mState+stateSize, sizeof(int16_t)*stateSize);
        impulse -= stateSize;
    }
    int16_t* head = impulse + c->halfNumCoefs*CHANNELS;
    head[0] = in[inputIndex*CHANNELS + 0];
    if (CHANNELS == 2)
        head[1] = in[inputIndex*CHANNELS + 1];
}

template<int CHANNELS>
void AudioResamplerSinc::filterCoefficient(
        int32_t& l, int32_t& r, uint32_t phase, const int16_t *samples)
{
    const Constants *c = mConstants;

    // compute the index of the coefficient on the positive side and
    // negative side
    uint32_t indexP = (phase & c->cMask) >> c->cShift;
    uint16_t lerpP = (phase & c->pMask) >> c->pShift;
    uint32_t indexN = (-phase & c->cMask) >> c->cShift;
    uint16_t lerpN = (-phase & c->pMask) >> c->pShift;
    if ((indexP == 0) && (lerpP == 0)) {
        indexN = c->cMask >> c->cShift;
        lerpN = c->pMask >> c->pShift;
    }

    l = 0;
    r = 0;
    const int32_t* coefs = mFirCoefs;
    const int16_t *sP = samples;
    const int16_t *sN = samples+CHANNELS;
    for (unsigned int i=0 ; i < c->halfNumCoefs/4 ; i++) {
        interpolate<CHANNELS>(l, r, coefs+indexP, lerpP, sP);
        interpolate<CHANNELS>(l, r, coefs+indexN, lerpN, sN);
        sP -= CHANNELS; sN += CHANNELS; coefs += 1 << c->coefsBits;
        interpolate<CHANNELS>(l, r, coefs+indexP, lerpP, sP);
        interpolate<CHANNELS>(l, r, coefs+indexN, lerpN, sN);
        sP -= CHANNELS; sN += CHANNELS; coefs += 1 << c->coefsBits;
        interpolate<CHANNELS>(l, r, coefs+indexP, lerpP, sP);
        interpolate<CHANNELS>(l, r, coefs+indexN, lerpN, sN);
        sP -= CHANNELS; sN += CHANNELS; coefs += 1 << c->coefsBits;
        interpolate<CHANNELS>(l, r, coefs+indexP, lerpP, sP);
        interpolate<CHANNELS>(l, r, coefs+indexN, lerpN, sN);
        sP -= CHANNELS; sN += CHANNELS; coefs += 1 << c->coefsBits;
    }
}

template<int CHANNELS>
void AudioResamplerSinc::interpolate(
        int32_t& l, int32_t& r,
        const int32_t* coefs, int16_t lerp, const int16_t* samples)
{
    int32_t c0 = coefs[0];
    int32_t c1 = coefs[1];
    int32_t sinc = mulAdd(lerp, (c1-c0)<<1, c0);
    if (CHANNELS == 2) {
        uint32_t rl = *reinterpret_cast<const uint32_t*>(samples);
        l = mulAddRL(1, rl, sinc, l);
        r = mulAddRL(0, rl, sinc, r);
    } else {
        r = l = mulAdd(samples[0], sinc, l);
    }
}
// ----------------------------------------------------------------------------
}; // namespace android
