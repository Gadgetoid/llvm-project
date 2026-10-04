# RUN: llvm-mc -triple=sh3el-unknown-none-wince -show-encoding %s | FileCheck %s

# CHECK: mov r1, r2 ! encoding: [0x13,0x62]
mov r1,r2
# CHECK: mov #-1, r3 ! encoding: [0xff,0xe3]
mov #-1,r3
# CHECK: mov.b @r1, r2 ! encoding: [0x10,0x62]
mov.b @r1,r2
# CHECK: mov.w @r1, r2 ! encoding: [0x11,0x62]
mov.w @r1,r2
# CHECK: mov.l @r1, r2 ! encoding: [0x12,0x62]
mov.l @r1,r2
# CHECK: mov.l @r15+, r14 ! encoding: [0xf6,0x6e]
mov.l @r15+,r14
# CHECK: mov.l r14, @-r15 ! encoding: [0xe6,0x2f]
mov.l r14,@-r15
# CHECK: mov.b r1, @r2 ! encoding: [0x10,0x22]
mov.b r1,@r2
# CHECK: mov.l @(4, r1), r2 ! encoding: [0x11,0x52]
mov.l @(4,r1),r2
# CHECK: mov.l r2, @(60, r3) ! encoding: [0x2f,0x13]
mov.l r2,@(60,r3)
# CHECK: mov.b @(1, r4), r0 ! encoding: [0x41,0x84]
mov.b @(1,r4),r0
# CHECK: mov.w r0, @(2, r5) ! encoding: [0x51,0x81]
mov.w r0,@(2,r5)
# CHECK: mov.l @(r0, r5), r6 ! encoding: [0x5e,0x06]
mov.l @(r0,r5),r6
# CHECK: mov.b r1, @(r0, r5) ! encoding: [0x14,0x05]
mov.b r1,@(r0,r5)
# CHECK: mov.l @(8, gbr), r0 ! encoding: [0x02,0xc6]
mov.l @(8,gbr),r0
# CHECK: movt r9 ! encoding: [0x29,0x09]
movt r9
# CHECK: swap.w r1, r2 ! encoding: [0x19,0x62]
swap.w r1,r2
# CHECK: xtrct r1, r2 ! encoding: [0x1d,0x22]
xtrct r1,r2
# CHECK: add r1, r2 ! encoding: [0x1c,0x32]
add r1,r2
# CHECK: add #-128, r15 ! encoding: [0x80,0x7f]
add #-128,r15
# CHECK: addc r1, r2 ! encoding: [0x1e,0x32]
addc r1,r2
# CHECK: sub r1, r2 ! encoding: [0x18,0x32]
sub r1,r2
# CHECK: subc r1, r2 ! encoding: [0x1a,0x32]
subc r1,r2
# CHECK: neg r1, r2 ! encoding: [0x1b,0x62]
neg r1,r2
# CHECK: negc r1, r2 ! encoding: [0x1a,0x62]
negc r1,r2
# CHECK: cmp/eq #5, r0 ! encoding: [0x05,0x88]
cmp/eq #5,r0
# CHECK: cmp/eq r1, r2 ! encoding: [0x10,0x32]
cmp/eq r1,r2
# CHECK: cmp/hs r1, r2 ! encoding: [0x12,0x32]
cmp/hs r1,r2
# CHECK: cmp/ge r1, r2 ! encoding: [0x13,0x32]
cmp/ge r1,r2
# CHECK: cmp/hi r1, r2 ! encoding: [0x16,0x32]
cmp/hi r1,r2
# CHECK: cmp/gt r1, r2 ! encoding: [0x17,0x32]
cmp/gt r1,r2
# CHECK: cmp/pz r1 ! encoding: [0x11,0x41]
cmp/pz r1
# CHECK: cmp/pl r1 ! encoding: [0x15,0x41]
cmp/pl r1
# CHECK: tst r1, r1 ! encoding: [0x18,0x21]
tst r1,r1
# CHECK: tst #255, r0 ! encoding: [0xff,0xc8]
tst #255,r0
# CHECK: and r1, r2 ! encoding: [0x19,0x22]
and r1,r2
# CHECK: and #255, r0 ! encoding: [0xff,0xc9]
and #255,r0
# CHECK: or #1, r0 ! encoding: [0x01,0xcb]
or #1,r0
# CHECK: xor r1, r2 ! encoding: [0x1a,0x22]
xor r1,r2
# CHECK: not r1, r2 ! encoding: [0x17,0x62]
not r1,r2
# CHECK: exts.b r1, r2 ! encoding: [0x1e,0x62]
exts.b r1,r2
# CHECK: exts.w r1, r2 ! encoding: [0x1f,0x62]
exts.w r1,r2
# CHECK: extu.b r1, r2 ! encoding: [0x1c,0x62]
extu.b r1,r2
# CHECK: extu.w r1, r2 ! encoding: [0x1d,0x62]
extu.w r1,r2
# CHECK: mul.l r1, r2 ! encoding: [0x17,0x02]
mul.l r1,r2
# CHECK: dmuls.l r1, r2 ! encoding: [0x1d,0x32]
dmuls.l r1,r2
# CHECK: dmulu.l r1, r2 ! encoding: [0x15,0x32]
dmulu.l r1,r2
# CHECK: shll r1 ! encoding: [0x00,0x41]
shll r1
# CHECK: shlr r1 ! encoding: [0x01,0x41]
shlr r1
# CHECK: shar r1 ! encoding: [0x21,0x41]
shar r1
# CHECK: shll2 r1 ! encoding: [0x08,0x41]
shll2 r1
# CHECK: shlr16 r1 ! encoding: [0x29,0x41]
shlr16 r1
# CHECK: shad r1, r2 ! encoding: [0x1c,0x42]
shad r1,r2
# CHECK: shld r1, r2 ! encoding: [0x1d,0x42]
shld r1,r2
# CHECK: jmp @r0 ! encoding: [0x2b,0x40]
jmp @r0
# CHECK: jsr @r1 ! encoding: [0x0b,0x41]
jsr @r1
# CHECK: rts ! encoding: [0x0b,0x00]
rts
# CHECK: nop ! encoding: [0x09,0x00]
nop
# CHECK: clrt ! encoding: [0x08,0x00]
clrt
# CHECK: sts.l pr, @-r15 ! encoding: [0x22,0x4f]
sts.l pr,@-r15
# CHECK: lds.l @r15+, pr ! encoding: [0x26,0x4f]
lds.l @r15+,pr
# CHECK: sts macl, r0 ! encoding: [0x1a,0x00]
sts macl,r0
# CHECK: ldc r1, gbr ! encoding: [0x1e,0x41]
ldc r1,gbr
# CHECK: stc gbr, r1 ! encoding: [0x12,0x01]
stc gbr,r1
# CHECK: trapa #1 ! encoding: [0x01,0xc3]
trapa #1
