	.file	"modified_smith_waterman.c"
	.option nopic
	.attribute arch, "rv32i2p1_m2p0_a2p1_f2p2_d2p2_c2p0_zicsr2p0_zifencei2p0"
	.attribute unaligned_access, 0
	.attribute stack_align, 16
# GNU C17 () version 14.2.0 (riscv32-unknown-linux-gnu)
#	compiled by GNU C version 11.4.0, GMP version 6.2.1, MPFR version 4.1.0, MPC version 1.2.1, isl version none
# GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
# options passed: -mtune=rocket -mabi=ilp32d -misa-spec=20191213 -mtls-dialect=trad -march=rv32imafdc_zicsr_zifencei -g -O2
	.text
.Ltext0:
	.file 0 "/home/jitesh/Desktop/work/capstone/parallel_programming/shared_mem_modelling/smith_waterman" "modified_smith_waterman.c"
	.align	1
	.globl	smith_waterman
	.type	smith_waterman, @function
smith_waterman:
.LFB11:
	.file 1 "modified_smith_waterman.c"
	.loc 1 25 57
	.cfi_startproc
.LVL0:
	.loc 1 26 5
# modified_smith_waterman.c:25: void smith_waterman(const char* seq1, const char* seq2) {
	.loc 1 25 57 is_stmt 0
	addi	sp,sp,-32	#,,
	.cfi_def_cfa_offset 32
	sw	ra,28(sp)	#,
	sw	s0,24(sp)	#,
	sw	s1,20(sp)	#,
	.cfi_offset 1, -4
	.cfi_offset 8, -8
	.cfi_offset 9, -12
	mv	s0,a1	# seq2, tmp216
	sw	s2,16(sp)	#,
	sw	s3,12(sp)	#,
	sw	s4,8(sp)	#,
	sw	s5,4(sp)	#,
	sw	s6,0(sp)	#,
	.cfi_offset 18, -16
	.cfi_offset 19, -20
	.cfi_offset 20, -24
	.cfi_offset 21, -28
	.cfi_offset 22, -32
# modified_smith_waterman.c:25: void smith_waterman(const char* seq1, const char* seq2) {
	.loc 1 25 57
	mv	s6,a0	# seq1, tmp215
# modified_smith_waterman.c:26:     int len1 = strlen(seq1);
	.loc 1 26 16
	call	strlen		#
.LVL1:
	mv	s2,a0	# tmp217,
# modified_smith_waterman.c:27:     int len2 = strlen(seq2);
	.loc 1 27 16
	mv	a0,s0	#, seq2
.LVL2:
	.loc 1 27 5 is_stmt 1
# modified_smith_waterman.c:27:     int len2 = strlen(seq2);
	.loc 1 27 16 is_stmt 0
	call	strlen		#
.LVL3:
.LBB2:
.LBB3:
# modified_smith_waterman.c:32:             dp_matrix[i][j] = 0;
	.loc 1 32 29
	addi	s1,a0,1	#, _90, _2
	lui	a5,%hi(dp_matrix)	# tmp178,
.LBE3:
.LBE2:
# modified_smith_waterman.c:27:     int len2 = strlen(seq2);
	.loc 1 27 16
	mv	s4,a0	# _2, tmp218
.LVL4:
	.loc 1 30 5 is_stmt 1
.LBB6:
	.loc 1 30 10
	.loc 1 30 23 discriminator 1
.LBB4:
# modified_smith_waterman.c:32:             dp_matrix[i][j] = 0;
	.loc 1 32 29 is_stmt 0
	slli	s5,s1,1	#, _30, _90
	addi	a5,a5,%lo(dp_matrix)	# ivtmp.45, tmp178,
.LBE4:
# modified_smith_waterman.c:30:     for (int i = 0; i <= len1; i++) {
	.loc 1 30 14
	li	s3,0		# i,
.LVL5:
.L2:
.LBB5:
	.loc 1 31 27 is_stmt 1 discriminator 1
# modified_smith_waterman.c:32:             dp_matrix[i][j] = 0;
	.loc 1 32 29 is_stmt 0
	mv	a0,a5	#, ivtmp.45
	mv	a2,s5	#, _30
	li	a1,0		#,
	call	memset		#
.LVL6:
.LBE5:
	.loc 1 30 33 is_stmt 1 discriminator 2
	addi	s3,s3,1	#, i, i
.LVL7:
	.loc 1 30 23 discriminator 1
	addi	a5,a0,258	#, ivtmp.45, ivtmp.45
	bge	s2,s3,.L2	#, _1, i,
.LVL8:
.LBE6:
.LBB7:
	.loc 1 37 23 discriminator 1
	beq	s2,zero,.L1	#, _1,,
	lui	t0,%hi(dp_matrix+260)	# tmp183,
	mv	t3,s6	# ivtmp.41, seq1
	addi	s2,s2,1	#, _106, _1
.LVL9:
	addi	t0,t0,%lo(dp_matrix+260)	# ivtmp.39, tmp183,
	li	t4,0		# ivtmp.36,
	li	t5,-258		# ivtmp.34,
# modified_smith_waterman.c:37:     for (int i = 1; i <= len1; i++) {
	.loc 1 37 14 is_stmt 0
	li	t6,1		# i,
.LVL10:
.LBB8:
	.loc 1 38 27 is_stmt 1 discriminator 1
	bne	s4,zero,.L23	#, _2,,
.L20:
.LBE8:
	.loc 1 37 33 discriminator 2
	addi	t6,t6,1	#, i, i
.LVL11:
	.loc 1 37 23 discriminator 1
	bne	t6,s2,.L20	#, i, _106,
.LVL12:
.L1:
.LBE7:
# modified_smith_waterman.c:52: }
	.loc 1 52 1 is_stmt 0
	lw	ra,28(sp)		#,
	.cfi_remember_state
	.cfi_restore 1
	lw	s0,24(sp)		#,
	.cfi_restore 8
.LVL13:
	lw	s1,20(sp)		#,
	.cfi_restore 9
.LVL14:
	lw	s2,16(sp)		#,
	.cfi_restore 18
	lw	s3,12(sp)		#,
	.cfi_restore 19
	lw	s4,8(sp)		#,
	.cfi_restore 20
	lw	s5,4(sp)		#,
	.cfi_restore 21
	lw	s6,0(sp)		#,
	.cfi_restore 22
	addi	sp,sp,32	#,,
	.cfi_def_cfa_offset 0
	jr	ra		#
.LVL15:
.L23:
	.cfi_restore_state
.LBB15:
.LBB13:
.LBB9:
# modified_smith_waterman.c:39:             uint16_t match = dp_matrix[i-1][j-1] + (seq1[i-1] == seq2[j-1] ? MATCH_SCORE : MISMATCH_SCORE);
	.loc 1 39 50 discriminator 2
	li	a1,65536		# tmp175,
	addi	a1,a1,-1	#, iftmp.0_70, tmp175
	lui	t1,%hi(max_score)	# tmp205,
# modified_smith_waterman.c:47:                 max_i = i;
	.loc 1 47 23
	lui	a0,%hi(max_i)	# tmp209,
# modified_smith_waterman.c:48:                 max_j = j;
	.loc 1 48 23
	lui	t2,%hi(max_j)	# tmp210,
.LVL16:
.L10:
	lhu	a6,-2(t0)	# dp_matrix_I_I_lsm0.16, MEM[(short unsigned int *)_110 + 4294967294B]
	lhu	a7,-260(t0)	# dp_matrix_I_I_lsm0.17, MEM[(short unsigned int *)_110 + 4294967036B]
.LBE9:
.LBE13:
.LBE15:
.LBB16:
# modified_smith_waterman.c:30:     for (int i = 0; i <= len1; i++) {
	.loc 1 30 14
	mv	a2,t0	# ivtmp.22, ivtmp.39
.LBE16:
.LBB17:
.LBB14:
# modified_smith_waterman.c:38:         for (int j = 1; j <= len2; j++) {
	.loc 1 38 18
	li	a3,1		# j,
.L9:
.LVL17:
.LBB10:
	.loc 1 39 13 is_stmt 1
	.loc 1 46 17
.LBE10:
	.loc 1 38 37 discriminator 2
.LBB11:
# modified_smith_waterman.c:39:             uint16_t match = dp_matrix[i-1][j-1] + (seq1[i-1] == seq2[j-1] ? MATCH_SCORE : MISMATCH_SCORE);
	.loc 1 39 70 is_stmt 0
	add	a4,s0,a3	# j, _89, seq2
# modified_smith_waterman.c:39:             uint16_t match = dp_matrix[i-1][j-1] + (seq1[i-1] == seq2[j-1] ? MATCH_SCORE : MISMATCH_SCORE);
	.loc 1 39 50
	lbu	s6,-1(a4)	# MEM[(const char *)_89 + 4294967295B], MEM[(const char *)_89 + 4294967295B]
	lbu	s5,0(t3)	# MEM[(const char *)_108], MEM[(const char *)_108]
# modified_smith_waterman.c:41:             uint16_t insert = dp_matrix[i][j-1] + GAP_PENALTY;
	.loc 1 41 22
	addi	a5,a6,-2	#, tmp192, dp_matrix_I_I_lsm0.16
# modified_smith_waterman.c:42:             dp_matrix[i][j] = MAX3(0, match, MAX(delete, insert));
	.loc 1 42 31
	slli	s4,a5,16	#, _61, tmp192
# modified_smith_waterman.c:40:             uint16_t delete = dp_matrix[i-1][j] + GAP_PENALTY;
	.loc 1 40 45
	add	a6,a2,t5	# ivtmp.34, _95, ivtmp.22
# modified_smith_waterman.c:45:             if (dp_matrix[i][j] > max_score) {
	.loc 1 45 16
	lhu	s3,%lo(max_score)(t1)	# max_score, max_score
# modified_smith_waterman.c:42:             dp_matrix[i][j] = MAX3(0, match, MAX(delete, insert));
	.loc 1 42 31
	srli	s4,s4,16	#, _61, _61
# modified_smith_waterman.c:40:             uint16_t delete = dp_matrix[i-1][j] + GAP_PENALTY;
	.loc 1 40 45
	add	a6,a6,t4	# ivtmp.36, _28, _95
# modified_smith_waterman.c:39:             uint16_t match = dp_matrix[i-1][j-1] + (seq1[i-1] == seq2[j-1] ? MATCH_SCORE : MISMATCH_SCORE);
	.loc 1 39 50 discriminator 2
	mv	a4,a1	# iftmp.0_70, iftmp.0_70
# modified_smith_waterman.c:39:             uint16_t match = dp_matrix[i-1][j-1] + (seq1[i-1] == seq2[j-1] ? MATCH_SCORE : MISMATCH_SCORE);
	.loc 1 39 50
	bne	s6,s5,.L5	#, MEM[(const char *)_89 + 4294967295B], MEM[(const char *)_108],
# modified_smith_waterman.c:39:             uint16_t match = dp_matrix[i-1][j-1] + (seq1[i-1] == seq2[j-1] ? MATCH_SCORE : MISMATCH_SCORE);
	.loc 1 39 50 discriminator 1
	li	a4,2		# iftmp.0_70,
.L5:
# modified_smith_waterman.c:39:             uint16_t match = dp_matrix[i-1][j-1] + (seq1[i-1] == seq2[j-1] ? MATCH_SCORE : MISMATCH_SCORE);
	.loc 1 39 22 discriminator 4
	add	a4,a7,a4	# iftmp.0_70, tmp187, dp_matrix_I_I_lsm0.17
	slli	a4,a4,16	#, match, tmp187
	srli	a4,a4,16	#, match, match
.LVL18:
	.loc 1 40 13 is_stmt 1
# modified_smith_waterman.c:40:             uint16_t delete = dp_matrix[i-1][j] + GAP_PENALTY;
	.loc 1 40 45 is_stmt 0
	lhu	a7,0(a6)	# dp_matrix_I_I_lsm0.17, MEM[(short unsigned int *)_28]
.LVL19:
	.loc 1 41 13 is_stmt 1
	.loc 1 42 13
# modified_smith_waterman.c:42:             dp_matrix[i][j] = MAX3(0, match, MAX(delete, insert));
	.loc 1 42 31 is_stmt 0
	bgeu	s4,a4,.L6	#, _61, match,
	mv	a5,a4	# _61, match
.L6:
# modified_smith_waterman.c:40:             uint16_t delete = dp_matrix[i-1][j] + GAP_PENALTY;
	.loc 1 40 22
	addi	a4,a7,-2	#, tmp196, dp_matrix_I_I_lsm0.17
.LVL20:
# modified_smith_waterman.c:42:             dp_matrix[i][j] = MAX3(0, match, MAX(delete, insert));
	.loc 1 42 31
	slli	s4,a5,16	#, _77, _77
	slli	a6,a4,16	#, tmp199, tmp196
	srli	s4,s4,16	#, _77, _77
	srli	a6,a6,16	#, tmp199, tmp199
	bgeu	s4,a6,.L7	#, _77, tmp199,
	mv	a5,a4	# _77, tmp196
.L7:
	slli	a6,a5,16	#, dp_matrix_I_I_lsm0.16, _77
# modified_smith_waterman.c:42:             dp_matrix[i][j] = MAX3(0, match, MAX(delete, insert));
	.loc 1 42 29
	sh	a5,0(a2)	# _77, MEM[(short unsigned int *)_92]
	.loc 1 45 13 is_stmt 1
# modified_smith_waterman.c:42:             dp_matrix[i][j] = MAX3(0, match, MAX(delete, insert));
	.loc 1 42 31 is_stmt 0
	srli	a6,a6,16	#, dp_matrix_I_I_lsm0.16, dp_matrix_I_I_lsm0.16
.LBE11:
# modified_smith_waterman.c:38:         for (int j = 1; j <= len2; j++) {
	.loc 1 38 27 discriminator 1
	addi	a2,a2,2	#, ivtmp.22, ivtmp.22
.LBB12:
# modified_smith_waterman.c:45:             if (dp_matrix[i][j] > max_score) {
	.loc 1 45 16
	bgeu	s3,a6,.L8	#, max_score, dp_matrix_I_I_lsm0.16,
# modified_smith_waterman.c:46:                 max_score = dp_matrix[i][j];
	.loc 1 46 27
	sh	a5,%lo(max_score)(t1)	# _77, max_score
	.loc 1 47 17 is_stmt 1
# modified_smith_waterman.c:47:                 max_i = i;
	.loc 1 47 23 is_stmt 0
	sh	t6,%lo(max_i)(a0)	# i, max_i
	.loc 1 48 17 is_stmt 1
# modified_smith_waterman.c:48:                 max_j = j;
	.loc 1 48 23 is_stmt 0
	sh	a3,%lo(max_j)(t2)	# j, max_j
.L8:
.LBE12:
# modified_smith_waterman.c:38:         for (int j = 1; j <= len2; j++) {
	.loc 1 38 37 discriminator 2
	addi	a3,a3,1	#, j, j
.LVL21:
	.loc 1 38 27 is_stmt 1 discriminator 1
	bne	a3,s1,.L9	#, j, _90,
.LBE14:
	.loc 1 37 33 discriminator 2
	addi	t6,t6,1	#, i, i
.LVL22:
	.loc 1 37 23 discriminator 1
	addi	t5,t5,-258	#, ivtmp.34, ivtmp.34
	addi	t4,t4,258	#, ivtmp.36, ivtmp.36
	addi	t0,t0,258	#, ivtmp.39, ivtmp.39
	addi	t3,t3,1	#, ivtmp.41, ivtmp.41
	bne	t6,s2,.L10	#, i, _106,
	j	.L1		#
.LBE17:
	.cfi_endproc
.LFE11:
	.size	smith_waterman, .-smith_waterman
	.section	.rodata.str1.4,"aMS",@progbits,1
	.align	2
.LC0:
	.string	"ACGAATCCGGGACGCCGCAAGTTAGGAGCCTAAGTTGTTGCATA"
	.align	2
.LC1:
	.string	"GGGGCCCCCCCCCCCCCCCCCCCCCCCTTTTTTTTTTTTTTTCCCCCCCCCCC"
	.align	2
.LC2:
	.string	"Max Score: %d at position (%d, %d)\n"
	.section	.text.startup,"ax",@progbits
	.align	1
	.globl	main
	.type	main, @function
main:
.LFB12:
	.loc 1 54 12
	.cfi_startproc
	.loc 1 55 5
.LVL23:
	.loc 1 56 5
	.loc 1 58 5
	lui	a1,%hi(.LC0)	# tmp141,
	lui	a0,%hi(.LC1)	# tmp142,
# modified_smith_waterman.c:54: int main() {
	.loc 1 54 12 is_stmt 0
	addi	sp,sp,-16	#,,
	.cfi_def_cfa_offset 16
# modified_smith_waterman.c:58:     smith_waterman(seq1, seq2);
	.loc 1 58 5
	addi	a1,a1,%lo(.LC0)	#, tmp141,
	addi	a0,a0,%lo(.LC1)	#, tmp142,
# modified_smith_waterman.c:54: int main() {
	.loc 1 54 12
	sw	ra,12(sp)	#,
	.cfi_offset 1, -4
# modified_smith_waterman.c:58:     smith_waterman(seq1, seq2);
	.loc 1 58 5
	call	smith_waterman		#
.LVL24:
	.loc 1 60 5 is_stmt 1
	lui	a4,%hi(max_i)	# tmp145,
	lui	a5,%hi(max_score)	# tmp147,
	lui	a3,%hi(max_j)	# tmp143,
	lhu	a3,%lo(max_j)(a3)	#, max_j
	lhu	a2,%lo(max_i)(a4)	#, max_i
	lhu	a1,%lo(max_score)(a5)	#, max_score
	lui	a0,%hi(.LC2)	# tmp149,
	addi	a0,a0,%lo(.LC2)	#, tmp149,
	call	printf		#
.LVL25:
	.loc 1 62 5
# modified_smith_waterman.c:63: }
	.loc 1 63 1 is_stmt 0
	lw	ra,12(sp)		#,
	.cfi_restore 1
	li	a0,0		#,
	addi	sp,sp,16	#,,
	.cfi_def_cfa_offset 0
	jr	ra		#
	.cfi_endproc
.LFE12:
	.size	main, .-main
	.globl	dp_matrix
	.globl	max_j
	.globl	max_i
	.globl	max_score
	.bss
	.align	2
	.type	dp_matrix, @object
	.size	dp_matrix, 264450
dp_matrix:
	.zero	264450
	.section	.sbss,"aw",@nobits
	.align	1
	.type	max_j, @object
	.size	max_j, 2
max_j:
	.zero	2
	.type	max_i, @object
	.size	max_i, 2
max_i:
	.zero	2
	.type	max_score, @object
	.size	max_score, 2
max_score:
	.zero	2
	.text
.Letext0:
	.file 2 "/home/jitesh/binaries/risc32_install/sysroot/usr/include/bits/types.h"
	.file 3 "/home/jitesh/binaries/risc32_install/sysroot/usr/include/bits/stdint-uintn.h"
	.file 4 "/home/jitesh/binaries/risc32_install/lib/gcc/riscv32-unknown-linux-gnu/14.2.0/include/stddef.h"
	.file 5 "/home/jitesh/binaries/risc32_install/sysroot/usr/include/stdio.h"
	.file 6 "/home/jitesh/binaries/risc32_install/sysroot/usr/include/string.h"
	.file 7 "<built-in>"
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.4byte	0x2b0
	.2byte	0x5
	.byte	0x1
	.byte	0x4
	.4byte	.Ldebug_abbrev0
	.uleb128 0xf
	.4byte	.LASF29
	.byte	0x1d
	.4byte	.LASF0
	.4byte	.LASF1
	.4byte	.LLRL14
	.4byte	0
	.4byte	.Ldebug_line0
	.uleb128 0x1
	.byte	0x8
	.byte	0x7
	.4byte	.LASF2
	.uleb128 0x1
	.byte	0x4
	.byte	0x7
	.4byte	.LASF3
	.uleb128 0x1
	.byte	0x1
	.byte	0x8
	.4byte	.LASF4
	.uleb128 0x1
	.byte	0x2
	.byte	0x7
	.4byte	.LASF5
	.uleb128 0x1
	.byte	0x4
	.byte	0x7
	.4byte	.LASF6
	.uleb128 0x1
	.byte	0x1
	.byte	0x6
	.4byte	.LASF7
	.uleb128 0x1
	.byte	0x2
	.byte	0x5
	.4byte	.LASF8
	.uleb128 0x5
	.4byte	.LASF12
	.byte	0x2
	.byte	0x28
	.byte	0x1c
	.4byte	0x3b
	.uleb128 0x10
	.byte	0x4
	.byte	0x5
	.string	"int"
	.uleb128 0x1
	.byte	0x8
	.byte	0x5
	.4byte	.LASF9
	.uleb128 0x1
	.byte	0x4
	.byte	0x5
	.4byte	.LASF10
	.uleb128 0x1
	.byte	0x1
	.byte	0x8
	.4byte	.LASF11
	.uleb128 0x11
	.4byte	0x78
	.uleb128 0x5
	.4byte	.LASF13
	.byte	0x3
	.byte	0x19
	.byte	0x14
	.4byte	0x57
	.uleb128 0x5
	.4byte	.LASF14
	.byte	0x4
	.byte	0xd6
	.byte	0x17
	.4byte	0x2d
	.uleb128 0x12
	.byte	0x4
	.4byte	0x7f
	.uleb128 0x3
	.4byte	.LASF15
	.byte	0xc
	.4byte	0x84
	.uleb128 0x5
	.byte	0x3
	.4byte	max_score
	.uleb128 0x3
	.4byte	.LASF16
	.byte	0xd
	.4byte	0x84
	.uleb128 0x5
	.byte	0x3
	.4byte	max_i
	.uleb128 0x3
	.4byte	.LASF17
	.byte	0xe
	.4byte	0x84
	.uleb128 0x5
	.byte	0x3
	.4byte	max_j
	.uleb128 0x13
	.4byte	0x84
	.4byte	0xe9
	.uleb128 0x14
	.4byte	0x2d
	.2byte	0x400
	.uleb128 0x15
	.4byte	0x2d
	.byte	0x80
	.byte	0
	.uleb128 0x3
	.4byte	.LASF18
	.byte	0x16
	.4byte	0xd2
	.uleb128 0x5
	.byte	0x3
	.4byte	dp_matrix
	.uleb128 0xa
	.4byte	.LASF19
	.byte	0x5
	.2byte	0x16b
	.byte	0xc
	.4byte	0x63
	.4byte	0x111
	.uleb128 0xb
	.4byte	0x9c
	.uleb128 0x16
	.byte	0
	.uleb128 0xa
	.4byte	.LASF20
	.byte	0x6
	.2byte	0x197
	.byte	0xf
	.4byte	0x90
	.4byte	0x128
	.uleb128 0xb
	.4byte	0x9c
	.byte	0
	.uleb128 0x17
	.4byte	.LASF23
	.byte	0x1
	.byte	0x36
	.byte	0x5
	.4byte	0x63
	.4byte	.LFB12
	.4byte	.LFE12-.LFB12
	.uleb128 0x1
	.byte	0x9c
	.4byte	0x19a
	.uleb128 0x6
	.4byte	.LASF21
	.byte	0x37
	.byte	0x11
	.4byte	0x9c
	.uleb128 0x6
	.byte	0x3
	.4byte	.LC1
	.byte	0x9f
	.uleb128 0x6
	.4byte	.LASF22
	.byte	0x38
	.byte	0x11
	.4byte	0x9c
	.uleb128 0x6
	.byte	0x3
	.4byte	.LC0
	.byte	0x9f
	.uleb128 0xc
	.4byte	.LVL24
	.4byte	0x19a
	.4byte	0x186
	.uleb128 0x2
	.uleb128 0x1
	.byte	0x5a
	.uleb128 0x5
	.byte	0x3
	.4byte	.LC1
	.uleb128 0x2
	.uleb128 0x1
	.byte	0x5b
	.uleb128 0x5
	.byte	0x3
	.4byte	.LC0
	.byte	0
	.uleb128 0x7
	.4byte	.LVL25
	.4byte	0xf9
	.uleb128 0x2
	.uleb128 0x1
	.byte	0x5a
	.uleb128 0x5
	.byte	0x3
	.4byte	.LC2
	.byte	0
	.byte	0
	.uleb128 0x18
	.4byte	.LASF24
	.byte	0x1
	.byte	0x19
	.byte	0x6
	.4byte	.LFB11
	.4byte	.LFE11-.LFB11
	.uleb128 0x1
	.byte	0x9c
	.4byte	0x2a8
	.uleb128 0xd
	.4byte	.LASF21
	.byte	0x21
	.4byte	0x9c
	.4byte	.LLST0
	.uleb128 0xd
	.4byte	.LASF22
	.byte	0x33
	.4byte	0x9c
	.4byte	.LLST1
	.uleb128 0x8
	.4byte	.LASF25
	.byte	0x1a
	.byte	0x9
	.4byte	0x63
	.4byte	.LLST2
	.uleb128 0x8
	.4byte	.LASF26
	.byte	0x1b
	.byte	0x9
	.4byte	0x63
	.4byte	.LLST3
	.uleb128 0xe
	.4byte	.LLRL4
	.4byte	0x229
	.uleb128 0x4
	.string	"i"
	.byte	0x1e
	.byte	0xe
	.4byte	0x63
	.4byte	.LLST5
	.uleb128 0x9
	.4byte	.LLRL6
	.uleb128 0x4
	.string	"j"
	.byte	0x1f
	.byte	0x12
	.4byte	0x63
	.4byte	.LLST7
	.uleb128 0x7
	.4byte	.LVL6
	.4byte	0x2a8
	.uleb128 0x2
	.uleb128 0x1
	.byte	0x5b
	.uleb128 0x1
	.byte	0x30
	.uleb128 0x2
	.uleb128 0x1
	.byte	0x5c
	.uleb128 0x2
	.byte	0x85
	.sleb128 0
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0xe
	.4byte	.LLRL8
	.4byte	0x283
	.uleb128 0x4
	.string	"i"
	.byte	0x25
	.byte	0xe
	.4byte	0x63
	.4byte	.LLST9
	.uleb128 0x9
	.4byte	.LLRL10
	.uleb128 0x4
	.string	"j"
	.byte	0x26
	.byte	0x12
	.4byte	0x63
	.4byte	.LLST11
	.uleb128 0x9
	.4byte	.LLRL12
	.uleb128 0x8
	.4byte	.LASF27
	.byte	0x27
	.byte	0x16
	.4byte	0x84
	.4byte	.LLST13
	.uleb128 0x6
	.4byte	.LASF28
	.byte	0x28
	.byte	0x16
	.4byte	0x84
	.uleb128 0x3
	.byte	0x81
	.sleb128 -2
	.byte	0x9f
	.uleb128 0x19
	.4byte	.LASF30
	.byte	0x1
	.byte	0x29
	.byte	0x16
	.4byte	0x84
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0xc
	.4byte	.LVL1
	.4byte	0x111
	.4byte	0x297
	.uleb128 0x2
	.uleb128 0x1
	.byte	0x5a
	.uleb128 0x2
	.byte	0x86
	.sleb128 0
	.byte	0
	.uleb128 0x7
	.4byte	.LVL3
	.4byte	0x111
	.uleb128 0x2
	.uleb128 0x1
	.byte	0x5a
	.uleb128 0x2
	.byte	0x78
	.sleb128 0
	.byte	0
	.byte	0
	.uleb128 0x1a
	.4byte	.LASF31
	.4byte	.LASF32
	.byte	0x7
	.byte	0
	.byte	0
	.section	.debug_abbrev,"",@progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x49
	.byte	0
	.uleb128 0x2
	.uleb128 0x18
	.uleb128 0x7e
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x3
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0x21
	.sleb128 1
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0x21
	.sleb128 10
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x4
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0x21
	.sleb128 1
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0x21
	.sleb128 1
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0x48
	.byte	0x1
	.uleb128 0x7d
	.uleb128 0x1
	.uleb128 0x7f
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0x21
	.sleb128 1
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x55
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3c
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xb
	.uleb128 0x5
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xc
	.uleb128 0x48
	.byte	0x1
	.uleb128 0x7d
	.uleb128 0x1
	.uleb128 0x7f
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xd
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0x21
	.sleb128 1
	.uleb128 0x3b
	.uleb128 0x21
	.sleb128 25
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0xe
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x55
	.uleb128 0x17
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xf
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x1f
	.uleb128 0x1b
	.uleb128 0x1f
	.uleb128 0x55
	.uleb128 0x17
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x10
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x10
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0
	.byte	0
	.uleb128 0x11
	.uleb128 0x26
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x12
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x13
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x14
	.uleb128 0x21
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0x5
	.byte	0
	.byte	0
	.uleb128 0x15
	.uleb128 0x21
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x16
	.uleb128 0x18
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x17
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x7a
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x18
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x7a
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x19
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x1a
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3c
	.uleb128 0x19
	.uleb128 0x6e
	.uleb128 0xe
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_loclists,"",@progbits
	.4byte	.Ldebug_loc3-.Ldebug_loc2
.Ldebug_loc2:
	.2byte	0x5
	.byte	0x4
	.byte	0
	.4byte	0
.Ldebug_loc0:
.LLST0:
	.byte	0x6
	.4byte	.LVL0
	.byte	0x4
	.uleb128 .LVL0-.LVL0
	.uleb128 .LVL1-1-.LVL0
	.uleb128 0x1
	.byte	0x5a
	.byte	0x4
	.uleb128 .LVL1-1-.LVL0
	.uleb128 .LVL12-.LVL0
	.uleb128 0x1
	.byte	0x66
	.byte	0x4
	.uleb128 .LVL12-.LVL0
	.uleb128 .LVL15-.LVL0
	.uleb128 0xa
	.byte	0xa3
	.uleb128 0x3
	.byte	0xa5
	.uleb128 0xa
	.uleb128 0x26
	.byte	0xa8
	.uleb128 0x2d
	.byte	0xa8
	.uleb128 0
	.byte	0x9f
	.byte	0x4
	.uleb128 .LVL15-.LVL0
	.uleb128 .LVL16-.LVL0
	.uleb128 0x1
	.byte	0x66
	.byte	0x4
	.uleb128 .LVL16-.LVL0
	.uleb128 .LFE11-.LVL0
	.uleb128 0xa
	.byte	0xa3
	.uleb128 0x3
	.byte	0xa5
	.uleb128 0xa
	.uleb128 0x26
	.byte	0xa8
	.uleb128 0x2d
	.byte	0xa8
	.uleb128 0
	.byte	0x9f
	.byte	0
.LLST1:
	.byte	0x6
	.4byte	.LVL0
	.byte	0x4
	.uleb128 .LVL0-.LVL0
	.uleb128 .LVL1-1-.LVL0
	.uleb128 0x1
	.byte	0x5b
	.byte	0x4
	.uleb128 .LVL1-1-.LVL0
	.uleb128 .LVL13-.LVL0
	.uleb128 0x1
	.byte	0x58
	.byte	0x4
	.uleb128 .LVL13-.LVL0
	.uleb128 .LVL15-.LVL0
	.uleb128 0xa
	.byte	0xa3
	.uleb128 0x3
	.byte	0xa5
	.uleb128 0xb
	.uleb128 0x26
	.byte	0xa8
	.uleb128 0x2d
	.byte	0xa8
	.uleb128 0
	.byte	0x9f
	.byte	0x4
	.uleb128 .LVL15-.LVL0
	.uleb128 .LFE11-.LVL0
	.uleb128 0x1
	.byte	0x58
	.byte	0
.LLST2:
	.byte	0x6
	.4byte	.LVL2
	.byte	0x4
	.uleb128 .LVL2-.LVL2
	.uleb128 .LVL9-.LVL2
	.uleb128 0x1
	.byte	0x62
	.byte	0x4
	.uleb128 .LVL9-.LVL2
	.uleb128 .LVL12-.LVL2
	.uleb128 0x3
	.byte	0x82
	.sleb128 -1
	.byte	0x9f
	.byte	0x4
	.uleb128 .LVL15-.LVL2
	.uleb128 .LFE11-.LVL2
	.uleb128 0x3
	.byte	0x82
	.sleb128 -1
	.byte	0x9f
	.byte	0
.LLST3:
	.byte	0x6
	.4byte	.LVL4
	.byte	0x4
	.uleb128 .LVL4-.LVL4
	.uleb128 .LVL5-.LVL4
	.uleb128 0x1
	.byte	0x5a
	.byte	0x4
	.uleb128 .LVL5-.LVL4
	.uleb128 .LVL12-.LVL4
	.uleb128 0x1
	.byte	0x64
	.byte	0x4
	.uleb128 .LVL12-.LVL4
	.uleb128 .LVL14-.LVL4
	.uleb128 0x3
	.byte	0x79
	.sleb128 -1
	.byte	0x9f
	.byte	0x4
	.uleb128 .LVL15-.LVL4
	.uleb128 .LVL16-.LVL4
	.uleb128 0x1
	.byte	0x64
	.byte	0x4
	.uleb128 .LVL16-.LVL4
	.uleb128 .LFE11-.LVL4
	.uleb128 0x3
	.byte	0x79
	.sleb128 -1
	.byte	0x9f
	.byte	0
.LLST5:
	.byte	0x6
	.4byte	.LVL4
	.byte	0x4
	.uleb128 .LVL4-.LVL4
	.uleb128 .LVL5-.LVL4
	.uleb128 0x2
	.byte	0x30
	.byte	0x9f
	.byte	0x4
	.uleb128 .LVL5-.LVL4
	.uleb128 .LVL12-.LVL4
	.uleb128 0x1
	.byte	0x63
	.byte	0x4
	.uleb128 .LVL15-.LVL4
	.uleb128 .LVL16-.LVL4
	.uleb128 0x1
	.byte	0x63
	.byte	0
.LLST7:
	.byte	0x8
	.4byte	.LVL5
	.uleb128 .LVL6-.LVL5
	.uleb128 0x2
	.byte	0x30
	.byte	0x9f
	.byte	0
.LLST9:
	.byte	0x6
	.4byte	.LVL8
	.byte	0x4
	.uleb128 .LVL8-.LVL8
	.uleb128 .LVL10-.LVL8
	.uleb128 0x2
	.byte	0x31
	.byte	0x9f
	.byte	0x4
	.uleb128 .LVL10-.LVL8
	.uleb128 .LVL12-.LVL8
	.uleb128 0x1
	.byte	0x6f
	.byte	0x4
	.uleb128 .LVL15-.LVL8
	.uleb128 .LVL16-.LVL8
	.uleb128 0x1
	.byte	0x6f
	.byte	0x4
	.uleb128 .LVL22-.LVL8
	.uleb128 .LFE11-.LVL8
	.uleb128 0x1
	.byte	0x6f
	.byte	0
.LLST11:
	.byte	0x6
	.4byte	.LVL10
	.byte	0x4
	.uleb128 .LVL10-.LVL10
	.uleb128 .LVL12-.LVL10
	.uleb128 0x2
	.byte	0x31
	.byte	0x9f
	.byte	0x4
	.uleb128 .LVL15-.LVL10
	.uleb128 .LVL16-.LVL10
	.uleb128 0x2
	.byte	0x31
	.byte	0x9f
	.byte	0x4
	.uleb128 .LVL17-.LVL10
	.uleb128 .LFE11-.LVL10
	.uleb128 0x1
	.byte	0x5d
	.byte	0
.LLST13:
	.byte	0x8
	.4byte	.LVL18
	.uleb128 .LVL20-.LVL18
	.uleb128 0x1
	.byte	0x5e
	.byte	0
.Ldebug_loc3:
	.section	.debug_aranges,"",@progbits
	.4byte	0x24
	.2byte	0x2
	.4byte	.Ldebug_info0
	.byte	0x4
	.byte	0
	.2byte	0
	.2byte	0
	.4byte	.Ltext0
	.4byte	.Letext0-.Ltext0
	.4byte	.LFB12
	.4byte	.LFE12-.LFB12
	.4byte	0
	.4byte	0
	.section	.debug_rnglists,"",@progbits
.Ldebug_ranges0:
	.4byte	.Ldebug_ranges3-.Ldebug_ranges2
.Ldebug_ranges2:
	.2byte	0x5
	.byte	0x4
	.byte	0
	.4byte	0
.LLRL4:
	.byte	0x5
	.4byte	.LBB2
	.byte	0x4
	.uleb128 .LBB2-.LBB2
	.uleb128 .LBE2-.LBB2
	.byte	0x4
	.uleb128 .LBB6-.LBB2
	.uleb128 .LBE6-.LBB2
	.byte	0x4
	.uleb128 .LBB16-.LBB2
	.uleb128 .LBE16-.LBB2
	.byte	0
.LLRL6:
	.byte	0x5
	.4byte	.LBB3
	.byte	0x4
	.uleb128 .LBB3-.LBB3
	.uleb128 .LBE3-.LBB3
	.byte	0x4
	.uleb128 .LBB4-.LBB3
	.uleb128 .LBE4-.LBB3
	.byte	0x4
	.uleb128 .LBB5-.LBB3
	.uleb128 .LBE5-.LBB3
	.byte	0
.LLRL8:
	.byte	0x5
	.4byte	.LBB7
	.byte	0x4
	.uleb128 .LBB7-.LBB7
	.uleb128 .LBE7-.LBB7
	.byte	0x4
	.uleb128 .LBB15-.LBB7
	.uleb128 .LBE15-.LBB7
	.byte	0x4
	.uleb128 .LBB17-.LBB7
	.uleb128 .LBE17-.LBB7
	.byte	0
.LLRL10:
	.byte	0x5
	.4byte	.LBB8
	.byte	0x4
	.uleb128 .LBB8-.LBB8
	.uleb128 .LBE8-.LBB8
	.byte	0x4
	.uleb128 .LBB13-.LBB8
	.uleb128 .LBE13-.LBB8
	.byte	0x4
	.uleb128 .LBB14-.LBB8
	.uleb128 .LBE14-.LBB8
	.byte	0
.LLRL12:
	.byte	0x5
	.4byte	.LBB9
	.byte	0x4
	.uleb128 .LBB9-.LBB9
	.uleb128 .LBE9-.LBB9
	.byte	0x4
	.uleb128 .LBB10-.LBB9
	.uleb128 .LBE10-.LBB9
	.byte	0x4
	.uleb128 .LBB11-.LBB9
	.uleb128 .LBE11-.LBB9
	.byte	0x4
	.uleb128 .LBB12-.LBB9
	.uleb128 .LBE12-.LBB9
	.byte	0
.LLRL14:
	.byte	0x7
	.4byte	.Ltext0
	.uleb128 .Letext0-.Ltext0
	.byte	0x7
	.4byte	.LFB12
	.uleb128 .LFE12-.LFB12
	.byte	0
.Ldebug_ranges3:
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.section	.debug_str,"MS",@progbits,1
.LASF14:
	.string	"size_t"
.LASF29:
	.string	"GNU C17 14.2.0 -mtune=rocket -mabi=ilp32d -misa-spec=20191213 -mtls-dialect=trad -march=rv32imafdc_zicsr_zifencei -g -O2"
.LASF24:
	.string	"smith_waterman"
.LASF15:
	.string	"max_score"
.LASF20:
	.string	"strlen"
.LASF4:
	.string	"unsigned char"
.LASF27:
	.string	"match"
.LASF18:
	.string	"dp_matrix"
.LASF32:
	.string	"__builtin_memset"
.LASF6:
	.string	"long unsigned int"
.LASF28:
	.string	"delete"
.LASF5:
	.string	"short unsigned int"
.LASF26:
	.string	"len2"
.LASF31:
	.string	"memset"
.LASF12:
	.string	"__uint16_t"
.LASF23:
	.string	"main"
.LASF22:
	.string	"seq2"
.LASF3:
	.string	"unsigned int"
.LASF2:
	.string	"long long unsigned int"
.LASF16:
	.string	"max_i"
.LASF17:
	.string	"max_j"
.LASF30:
	.string	"insert"
.LASF9:
	.string	"long long int"
.LASF11:
	.string	"char"
.LASF21:
	.string	"seq1"
.LASF19:
	.string	"printf"
.LASF8:
	.string	"short int"
.LASF25:
	.string	"len1"
.LASF13:
	.string	"uint16_t"
.LASF10:
	.string	"long int"
.LASF7:
	.string	"signed char"
	.section	.debug_line_str,"MS",@progbits,1
.LASF0:
	.string	"modified_smith_waterman.c"
.LASF1:
	.string	"/home/jitesh/Desktop/work/capstone/parallel_programming/shared_mem_modelling/smith_waterman"
	.ident	"GCC: () 14.2.0"
	.section	.note.GNU-stack,"",@progbits
