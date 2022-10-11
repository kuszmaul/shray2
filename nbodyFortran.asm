
bin/nbody_fortran:     file format elf64-x86-64


Disassembly of section .init:

0000000000001000 <_init>:
    1000:	f3 0f 1e fa          	endbr64 
    1004:	48 83 ec 08          	sub    $0x8,%rsp
    1008:	48 8b 05 c9 3f 00 00 	mov    0x3fc9(%rip),%rax        # 4fd8 <__gmon_start__@Base>
    100f:	48 85 c0             	test   %rax,%rax
    1012:	74 02                	je     1016 <_init+0x16>
    1014:	ff d0                	call   *%rax
    1016:	48 83 c4 08          	add    $0x8,%rsp
    101a:	c3                   	ret    

Disassembly of section .plt:

0000000000001020 <memset@plt-0x10>:
    1020:	ff 35 fa 3e 00 00    	push   0x3efa(%rip)        # 4f20 <_GLOBAL_OFFSET_TABLE_+0x8>
    1026:	ff 25 fc 3e 00 00    	jmp    *0x3efc(%rip)        # 4f28 <_GLOBAL_OFFSET_TABLE_+0x10>
    102c:	0f 1f 40 00          	nopl   0x0(%rax)

0000000000001030 <memset@plt>:
    1030:	ff 25 fa 3e 00 00    	jmp    *0x3efa(%rip)        # 4f30 <memset@GLIBC_2.2.5>
    1036:	68 00 00 00 00       	push   $0x0
    103b:	e9 e0 ff ff ff       	jmp    1020 <_init+0x20>

0000000000001040 <_gfortran_runtime_error_at@plt>:
    1040:	ff 25 f2 3e 00 00    	jmp    *0x3ef2(%rip)        # 4f38 <_gfortran_runtime_error_at@GFORTRAN_8>
    1046:	68 01 00 00 00       	push   $0x1
    104b:	e9 d0 ff ff ff       	jmp    1020 <_init+0x20>

0000000000001050 <_gfortran_st_read_done@plt>:
    1050:	ff 25 ea 3e 00 00    	jmp    *0x3eea(%rip)        # 4f40 <_gfortran_st_read_done@GFORTRAN_8>
    1056:	68 02 00 00 00       	push   $0x2
    105b:	e9 c0 ff ff ff       	jmp    1020 <_init+0x20>

0000000000001060 <malloc@plt>:
    1060:	ff 25 e2 3e 00 00    	jmp    *0x3ee2(%rip)        # 4f48 <malloc@GLIBC_2.2.5>
    1066:	68 03 00 00 00       	push   $0x3
    106b:	e9 b0 ff ff ff       	jmp    1020 <_init+0x20>

0000000000001070 <free@plt>:
    1070:	ff 25 da 3e 00 00    	jmp    *0x3eda(%rip)        # 4f50 <free@GLIBC_2.2.5>
    1076:	68 04 00 00 00       	push   $0x4
    107b:	e9 a0 ff ff ff       	jmp    1020 <_init+0x20>

0000000000001080 <_gfortran_transfer_integer@plt>:
    1080:	ff 25 d2 3e 00 00    	jmp    *0x3ed2(%rip)        # 4f58 <_gfortran_transfer_integer@GFORTRAN_8>
    1086:	68 05 00 00 00       	push   $0x5
    108b:	e9 90 ff ff ff       	jmp    1020 <_init+0x20>

0000000000001090 <_gfortran_transfer_character_write@plt>:
    1090:	ff 25 ca 3e 00 00    	jmp    *0x3eca(%rip)        # 4f60 <_gfortran_transfer_character_write@GFORTRAN_8>
    1096:	68 06 00 00 00       	push   $0x6
    109b:	e9 80 ff ff ff       	jmp    1020 <_init+0x20>

00000000000010a0 <_gfortran_os_error_at@plt>:
    10a0:	ff 25 c2 3e 00 00    	jmp    *0x3ec2(%rip)        # 4f68 <_gfortran_os_error_at@GFORTRAN_10>
    10a6:	68 07 00 00 00       	push   $0x7
    10ab:	e9 70 ff ff ff       	jmp    1020 <_init+0x20>

00000000000010b0 <_gfortran_caf_sync_all@plt>:
    10b0:	ff 25 ba 3e 00 00    	jmp    *0x3eba(%rip)        # 4f70 <_gfortran_caf_sync_all@Base>
    10b6:	68 08 00 00 00       	push   $0x8
    10bb:	e9 60 ff ff ff       	jmp    1020 <_init+0x20>

00000000000010c0 <_gfortran_st_read@plt>:
    10c0:	ff 25 b2 3e 00 00    	jmp    *0x3eb2(%rip)        # 4f78 <_gfortran_st_read@GFORTRAN_8>
    10c6:	68 09 00 00 00       	push   $0x9
    10cb:	e9 50 ff ff ff       	jmp    1020 <_init+0x20>

00000000000010d0 <_gfortran_internal_pack@plt>:
    10d0:	ff 25 aa 3e 00 00    	jmp    *0x3eaa(%rip)        # 4f80 <_gfortran_internal_pack@GFORTRAN_8>
    10d6:	68 0a 00 00 00       	push   $0xa
    10db:	e9 40 ff ff ff       	jmp    1020 <_init+0x20>

00000000000010e0 <_gfortran_st_write_done@plt>:
    10e0:	ff 25 a2 3e 00 00    	jmp    *0x3ea2(%rip)        # 4f88 <_gfortran_st_write_done@GFORTRAN_8>
    10e6:	68 0b 00 00 00       	push   $0xb
    10eb:	e9 30 ff ff ff       	jmp    1020 <_init+0x20>

00000000000010f0 <_gfortran_caf_num_images@plt>:
    10f0:	ff 25 9a 3e 00 00    	jmp    *0x3e9a(%rip)        # 4f90 <_gfortran_caf_num_images@Base>
    10f6:	68 0c 00 00 00       	push   $0xc
    10fb:	e9 20 ff ff ff       	jmp    1020 <_init+0x20>

0000000000001100 <_gfortran_set_options@plt>:
    1100:	ff 25 92 3e 00 00    	jmp    *0x3e92(%rip)        # 4f98 <_gfortran_set_options@GFORTRAN_8>
    1106:	68 0d 00 00 00       	push   $0xd
    110b:	e9 10 ff ff ff       	jmp    1020 <_init+0x20>

0000000000001110 <_gfortran_caf_register@plt>:
    1110:	ff 25 8a 3e 00 00    	jmp    *0x3e8a(%rip)        # 4fa0 <_gfortran_caf_register@Base>
    1116:	68 0e 00 00 00       	push   $0xe
    111b:	e9 00 ff ff ff       	jmp    1020 <_init+0x20>

0000000000001120 <_gfortran_get_command_argument_i4@plt>:
    1120:	ff 25 82 3e 00 00    	jmp    *0x3e82(%rip)        # 4fa8 <_gfortran_get_command_argument_i4@GFORTRAN_8>
    1126:	68 0f 00 00 00       	push   $0xf
    112b:	e9 f0 fe ff ff       	jmp    1020 <_init+0x20>

0000000000001130 <_gfortran_set_args@plt>:
    1130:	ff 25 7a 3e 00 00    	jmp    *0x3e7a(%rip)        # 4fb0 <_gfortran_set_args@GFORTRAN_8>
    1136:	68 10 00 00 00       	push   $0x10
    113b:	e9 e0 fe ff ff       	jmp    1020 <_init+0x20>

0000000000001140 <_gfortran_caf_finalize@plt>:
    1140:	ff 25 72 3e 00 00    	jmp    *0x3e72(%rip)        # 4fb8 <_gfortran_caf_finalize@Base>
    1146:	68 11 00 00 00       	push   $0x11
    114b:	e9 d0 fe ff ff       	jmp    1020 <_init+0x20>

0000000000001150 <_gfortran_caf_get@plt>:
    1150:	ff 25 6a 3e 00 00    	jmp    *0x3e6a(%rip)        # 4fc0 <_gfortran_caf_get@Base>
    1156:	68 12 00 00 00       	push   $0x12
    115b:	e9 c0 fe ff ff       	jmp    1020 <_init+0x20>

0000000000001160 <_gfortran_st_write@plt>:
    1160:	ff 25 62 3e 00 00    	jmp    *0x3e62(%rip)        # 4fc8 <_gfortran_st_write@GFORTRAN_8>
    1166:	68 13 00 00 00       	push   $0x13
    116b:	e9 b0 fe ff ff       	jmp    1020 <_init+0x20>

0000000000001170 <_gfortran_caf_init@plt>:
    1170:	ff 25 5a 3e 00 00    	jmp    *0x3e5a(%rip)        # 4fd0 <_gfortran_caf_init@Base>
    1176:	68 14 00 00 00       	push   $0x14
    117b:	e9 a0 fe ff ff       	jmp    1020 <_init+0x20>

Disassembly of section .plt.got:

0000000000001180 <__cxa_finalize@plt>:
    1180:	ff 25 6a 3e 00 00    	jmp    *0x3e6a(%rip)        # 4ff0 <__cxa_finalize@GLIBC_2.2.5>
    1186:	66 90                	xchg   %ax,%ax

Disassembly of section .text:

0000000000001190 <main>:
    1190:	48 83 ec 18          	sub    $0x18,%rsp
    1194:	89 7c 24 0c          	mov    %edi,0xc(%rsp)
    1198:	48 89 34 24          	mov    %rsi,(%rsp)
    119c:	48 8d 7c 24 0c       	lea    0xc(%rsp),%rdi
    11a1:	48 89 e6             	mov    %rsp,%rsi
    11a4:	e8 c7 ff ff ff       	call   1170 <_gfortran_caf_init@plt>
    11a9:	48 8b 34 24          	mov    (%rsp),%rsi
    11ad:	8b 7c 24 0c          	mov    0xc(%rsp),%edi
    11b1:	e8 7a ff ff ff       	call   1130 <_gfortran_set_args@plt>
    11b6:	48 8d 35 03 20 00 00 	lea    0x2003(%rip),%rsi        # 31c0 <options.103.4>
    11bd:	bf 07 00 00 00       	mov    $0x7,%edi
    11c2:	e8 39 ff ff ff       	call   1100 <_gfortran_set_options@plt>
    11c7:	e8 84 0c 00 00       	call   1e50 <MAIN__>
    11cc:	e8 6f ff ff ff       	call   1140 <_gfortran_caf_finalize@plt>
    11d1:	31 c0                	xor    %eax,%eax
    11d3:	48 83 c4 18          	add    $0x18,%rsp
    11d7:	c3                   	ret    
    11d8:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    11df:	00 

00000000000011e0 <set_fast_math>:
    11e0:	f3 0f 1e fa          	endbr64 
    11e4:	0f ae 5c 24 fc       	stmxcsr -0x4(%rsp)
    11e9:	81 4c 24 fc 40 80 00 	orl    $0x8040,-0x4(%rsp)
    11f0:	00 
    11f1:	0f ae 54 24 fc       	ldmxcsr -0x4(%rsp)
    11f6:	c3                   	ret    
    11f7:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    11fe:	00 00 

0000000000001200 <_start>:
    1200:	f3 0f 1e fa          	endbr64 
    1204:	31 ed                	xor    %ebp,%ebp
    1206:	49 89 d1             	mov    %rdx,%r9
    1209:	5e                   	pop    %rsi
    120a:	48 89 e2             	mov    %rsp,%rdx
    120d:	48 83 e4 f0          	and    $0xfffffffffffffff0,%rsp
    1211:	50                   	push   %rax
    1212:	54                   	push   %rsp
    1213:	45 31 c0             	xor    %r8d,%r8d
    1216:	31 c9                	xor    %ecx,%ecx
    1218:	48 8d 3d 71 ff ff ff 	lea    -0x8f(%rip),%rdi        # 1190 <main>
    121f:	ff 15 d3 3d 00 00    	call   *0x3dd3(%rip)        # 4ff8 <__libc_start_main@GLIBC_2.34>
    1225:	f4                   	hlt    
    1226:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    122d:	00 00 00 

0000000000001230 <deregister_tm_clones>:
    1230:	48 8d 3d d9 3d 00 00 	lea    0x3dd9(%rip),%rdi        # 5010 <__TMC_END__>
    1237:	48 8d 05 d2 3d 00 00 	lea    0x3dd2(%rip),%rax        # 5010 <__TMC_END__>
    123e:	48 39 f8             	cmp    %rdi,%rax
    1241:	74 15                	je     1258 <deregister_tm_clones+0x28>
    1243:	48 8b 05 96 3d 00 00 	mov    0x3d96(%rip),%rax        # 4fe0 <_ITM_deregisterTMCloneTable@Base>
    124a:	48 85 c0             	test   %rax,%rax
    124d:	74 09                	je     1258 <deregister_tm_clones+0x28>
    124f:	ff e0                	jmp    *%rax
    1251:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    1258:	c3                   	ret    
    1259:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)

0000000000001260 <register_tm_clones>:
    1260:	48 8d 3d a9 3d 00 00 	lea    0x3da9(%rip),%rdi        # 5010 <__TMC_END__>
    1267:	48 8d 35 a2 3d 00 00 	lea    0x3da2(%rip),%rsi        # 5010 <__TMC_END__>
    126e:	48 29 fe             	sub    %rdi,%rsi
    1271:	48 89 f0             	mov    %rsi,%rax
    1274:	48 c1 ee 3f          	shr    $0x3f,%rsi
    1278:	48 c1 f8 03          	sar    $0x3,%rax
    127c:	48 01 c6             	add    %rax,%rsi
    127f:	48 d1 fe             	sar    %rsi
    1282:	74 14                	je     1298 <register_tm_clones+0x38>
    1284:	48 8b 05 5d 3d 00 00 	mov    0x3d5d(%rip),%rax        # 4fe8 <_ITM_registerTMCloneTable@Base>
    128b:	48 85 c0             	test   %rax,%rax
    128e:	74 08                	je     1298 <register_tm_clones+0x38>
    1290:	ff e0                	jmp    *%rax
    1292:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
    1298:	c3                   	ret    
    1299:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)

00000000000012a0 <__do_global_dtors_aux>:
    12a0:	f3 0f 1e fa          	endbr64 
    12a4:	80 3d 75 3d 00 00 00 	cmpb   $0x0,0x3d75(%rip)        # 5020 <completed.0>
    12ab:	75 2b                	jne    12d8 <__do_global_dtors_aux+0x38>
    12ad:	55                   	push   %rbp
    12ae:	48 83 3d 3a 3d 00 00 	cmpq   $0x0,0x3d3a(%rip)        # 4ff0 <__cxa_finalize@GLIBC_2.2.5>
    12b5:	00 
    12b6:	48 89 e5             	mov    %rsp,%rbp
    12b9:	74 0c                	je     12c7 <__do_global_dtors_aux+0x27>
    12bb:	48 8b 3d 46 3d 00 00 	mov    0x3d46(%rip),%rdi        # 5008 <__dso_handle>
    12c2:	e8 b9 fe ff ff       	call   1180 <__cxa_finalize@plt>
    12c7:	e8 64 ff ff ff       	call   1230 <deregister_tm_clones>
    12cc:	c6 05 4d 3d 00 00 01 	movb   $0x1,0x3d4d(%rip)        # 5020 <completed.0>
    12d3:	5d                   	pop    %rbp
    12d4:	c3                   	ret    
    12d5:	0f 1f 00             	nopl   (%rax)
    12d8:	c3                   	ret    
    12d9:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)

00000000000012e0 <frame_dummy>:
    12e0:	f3 0f 1e fa          	endbr64 
    12e4:	e9 77 ff ff ff       	jmp    1260 <register_tm_clones>
    12e9:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)

00000000000012f0 <__acceleratemodule_MOD_accelerateall>:
    12f0:	4c 8d 54 24 08       	lea    0x8(%rsp),%r10
    12f5:	48 83 e4 e0          	and    $0xffffffffffffffe0,%rsp
    12f9:	41 ff 72 f8          	push   -0x8(%r10)
    12fd:	55                   	push   %rbp
    12fe:	48 89 e5             	mov    %rsp,%rbp
    1301:	41 57                	push   %r15
    1303:	41 56                	push   %r14
    1305:	41 55                	push   %r13
    1307:	41 54                	push   %r12
    1309:	41 52                	push   %r10
    130b:	53                   	push   %rbx
    130c:	48 81 ec a0 02 00 00 	sub    $0x2a0,%rsp
    1313:	49 8b 1a             	mov    (%r10),%rbx
    1316:	49 8b 42 18          	mov    0x18(%r10),%rax
    131a:	48 89 bd 50 fd ff ff 	mov    %rdi,-0x2b0(%rbp)
    1321:	48 89 b5 00 fe ff ff 	mov    %rsi,-0x200(%rbp)
    1328:	31 ff                	xor    %edi,%edi
    132a:	be ff ff ff ff       	mov    $0xffffffff,%esi
    132f:	48 89 95 f8 fd ff ff 	mov    %rdx,-0x208(%rbp)
    1336:	48 89 8d b0 fd ff ff 	mov    %rcx,-0x250(%rbp)
    133d:	48 89 9d f0 fd ff ff 	mov    %rbx,-0x210(%rbp)
    1344:	49 8b 5a 08          	mov    0x8(%r10),%rbx
    1348:	48 89 85 d8 fd ff ff 	mov    %rax,-0x228(%rbp)
    134f:	48 89 9d e8 fd ff ff 	mov    %rbx,-0x218(%rbp)
    1356:	49 8b 5a 10          	mov    0x10(%r10),%rbx
    135a:	48 89 9d e0 fd ff ff 	mov    %rbx,-0x220(%rbp)
    1361:	8b 19                	mov    (%rcx),%ebx
    1363:	e8 88 fd ff ff       	call   10f0 <_gfortran_caf_num_images@plt>
    1368:	be ff ff ff ff       	mov    $0xffffffff,%esi
    136d:	41 89 c0             	mov    %eax,%r8d
    1370:	89 d8                	mov    %ebx,%eax
    1372:	99                   	cltd   
    1373:	41 f7 f8             	idiv   %r8d
    1376:	4c 63 f8             	movslq %eax,%r15
    1379:	31 c0                	xor    %eax,%eax
    137b:	4d 85 ff             	test   %r15,%r15
    137e:	49 0f 49 c7          	cmovns %r15,%rax
    1382:	31 ff                	xor    %edi,%edi
    1384:	49 89 c5             	mov    %rax,%r13
    1387:	49 89 c4             	mov    %rax,%r12
    138a:	e8 61 fd ff ff       	call   10f0 <_gfortran_caf_num_images@plt>
    138f:	be ff ff ff ff       	mov    $0xffffffff,%esi
    1394:	41 89 c0             	mov    %eax,%r8d
    1397:	89 d8                	mov    %ebx,%eax
    1399:	49 f7 d5             	not    %r13
    139c:	99                   	cltd   
    139d:	41 f7 f8             	idiv   %r8d
    13a0:	48 63 d0             	movslq %eax,%rdx
    13a3:	89 85 50 fe ff ff    	mov    %eax,-0x1b0(%rbp)
    13a9:	31 c0                	xor    %eax,%eax
    13ab:	48 85 d2             	test   %rdx,%rdx
    13ae:	48 0f 49 c2          	cmovns %rdx,%rax
    13b2:	31 ff                	xor    %edi,%edi
    13b4:	48 89 85 48 fe ff ff 	mov    %rax,-0x1b8(%rbp)
    13bb:	48 f7 d0             	not    %rax
    13be:	49 89 c6             	mov    %rax,%r14
    13c1:	e8 2a fd ff ff       	call   10f0 <_gfortran_caf_num_images@plt>
    13c6:	4d 85 ff             	test   %r15,%r15
    13c9:	7e 59                	jle    1424 <__acceleratemodule_MOD_accelerateall+0x134>
    13cb:	48 8b bd 50 fd ff ff 	mov    -0x2b0(%rbp),%rdi
    13d2:	4e 8d 04 e5 00 00 00 	lea    0x0(,%r12,8),%r8
    13d9:	00 
    13da:	b9 03 00 00 00       	mov    $0x3,%ecx
    13df:	4c 89 b5 78 fe ff ff 	mov    %r14,-0x188(%rbp)
    13e6:	89 9d 70 fe ff ff    	mov    %ebx,-0x190(%rbp)
    13ec:	4d 89 ee             	mov    %r13,%r14
    13ef:	49 c1 e7 03          	shl    $0x3,%r15
    13f3:	4d 89 e5             	mov    %r12,%r13
    13f6:	48 89 cb             	mov    %rcx,%rbx
    13f9:	4d 89 c4             	mov    %r8,%r12
    13fc:	31 f6                	xor    %esi,%esi
    13fe:	4c 89 fa             	mov    %r15,%rdx
    1401:	e8 2a fc ff ff       	call   1030 <memset@plt>
    1406:	48 89 c7             	mov    %rax,%rdi
    1409:	4c 01 e7             	add    %r12,%rdi
    140c:	48 ff cb             	dec    %rbx
    140f:	75 eb                	jne    13fc <__acceleratemodule_MOD_accelerateall+0x10c>
    1411:	4d 89 ec             	mov    %r13,%r12
    1414:	8b 9d 70 fe ff ff    	mov    -0x190(%rbp),%ebx
    141a:	4d 89 f5             	mov    %r14,%r13
    141d:	4c 8b b5 78 fe ff ff 	mov    -0x188(%rbp),%r14
    1424:	be ff ff ff ff       	mov    $0xffffffff,%esi
    1429:	31 ff                	xor    %edi,%edi
    142b:	e8 c0 fc ff ff       	call   10f0 <_gfortran_caf_num_images@plt>
    1430:	89 85 4c fd ff ff    	mov    %eax,-0x2b4(%rbp)
    1436:	85 c0                	test   %eax,%eax
    1438:	0f 8e 14 06 00 00    	jle    1a52 <__acceleratemodule_MOD_accelerateall+0x762>
    143e:	48 8b 8d 50 fd ff ff 	mov    -0x2b0(%rbp),%rcx
    1445:	4b 8d 04 24          	lea    (%r12,%r12,1),%rax
    1449:	4a 8d 14 28          	lea    (%rax,%r13,1),%rdx
    144d:	4c 01 e0             	add    %r12,%rax
    1450:	48 8b bd 48 fe ff ff 	mov    -0x1b8(%rbp),%rdi
    1457:	41 bc 01 00 00 00    	mov    $0x1,%r12d
    145d:	4c 01 e8             	add    %r13,%rax
    1460:	48 8d 14 d1          	lea    (%rcx,%rdx,8),%rdx
    1464:	48 8d 04 c1          	lea    (%rcx,%rax,8),%rax
    1468:	48 8b 8d 00 fe ff ff 	mov    -0x200(%rbp),%rcx
    146f:	48 89 85 30 fd ff ff 	mov    %rax,-0x2d0(%rbp)
    1476:	48 89 95 38 fd ff ff 	mov    %rdx,-0x2c8(%rbp)
    147d:	48 8d 41 f8          	lea    -0x8(%rcx),%rax
    1481:	48 89 85 98 fd ff ff 	mov    %rax,-0x268(%rbp)
    1488:	48 8d 04 3f          	lea    (%rdi,%rdi,1),%rax
    148c:	4a 8d 14 30          	lea    (%rax,%r14,1),%rdx
    1490:	48 01 f8             	add    %rdi,%rax
    1493:	4c 01 f0             	add    %r14,%rax
    1496:	48 8d 14 d1          	lea    (%rcx,%rdx,8),%rdx
    149a:	48 8d 04 c1          	lea    (%rcx,%rax,8),%rax
    149e:	48 89 95 90 fd ff ff 	mov    %rdx,-0x270(%rbp)
    14a5:	48 89 85 88 fd ff ff 	mov    %rax,-0x278(%rbp)
    14ac:	be ff ff ff ff       	mov    $0xffffffff,%esi
    14b1:	31 ff                	xor    %edi,%edi
    14b3:	e8 38 fc ff ff       	call   10f0 <_gfortran_caf_num_images@plt>
    14b8:	41 89 c0             	mov    %eax,%r8d
    14bb:	89 d8                	mov    %ebx,%eax
    14bd:	99                   	cltd   
    14be:	41 f7 f8             	idiv   %r8d
    14c1:	8d 50 ff             	lea    -0x1(%rax),%edx
    14c4:	48 69 d2 1f 85 eb 51 	imul   $0x51eb851f,%rdx,%rdx
    14cb:	48 c1 ea 25          	shr    $0x25,%rdx
    14cf:	85 c0                	test   %eax,%eax
    14d1:	0f 8e 61 05 00 00    	jle    1a38 <__acceleratemodule_MOD_accelerateall+0x748>
    14d7:	8d 42 01             	lea    0x1(%rdx),%eax
    14da:	48 c7 85 80 fd ff ff 	movq   $0x0,-0x280(%rbp)
    14e1:	00 00 00 00 
    14e5:	48 6b c0 64          	imul   $0x64,%rax,%rax
    14e9:	48 89 85 40 fd ff ff 	mov    %rax,-0x2c0(%rbp)
    14f0:	4c 8b bd 80 fd ff ff 	mov    -0x280(%rbp),%r15
    14f7:	31 ff                	xor    %edi,%edi
    14f9:	be ff ff ff ff       	mov    $0xffffffff,%esi
    14fe:	44 89 f8             	mov    %r15d,%eax
    1501:	ff c0                	inc    %eax
    1503:	89 85 78 fd ff ff    	mov    %eax,-0x288(%rbp)
    1509:	e8 e2 fb ff ff       	call   10f0 <_gfortran_caf_num_images@plt>
    150e:	44 89 ff             	mov    %r15d,%edi
    1511:	41 89 c0             	mov    %eax,%r8d
    1514:	89 d8                	mov    %ebx,%eax
    1516:	83 c7 65             	add    $0x65,%edi
    1519:	99                   	cltd   
    151a:	89 bd 7c fd ff ff    	mov    %edi,-0x284(%rbp)
    1520:	41 f7 f8             	idiv   %r8d
    1523:	8d 50 ff             	lea    -0x1(%rax),%edx
    1526:	48 69 d2 1f 85 eb 51 	imul   $0x51eb851f,%rdx,%rdx
    152d:	48 c1 ea 25          	shr    $0x25,%rdx
    1531:	85 c0                	test   %eax,%eax
    1533:	0f 8e d9 04 00 00    	jle    1a12 <__acceleratemodule_MOD_accelerateall+0x722>
    1539:	48 8b 85 f8 fd ff ff 	mov    -0x208(%rbp),%rax
    1540:	6b d2 64             	imul   $0x64,%edx,%edx
    1543:	48 8b 8d 50 fd ff ff 	mov    -0x2b0(%rbp),%rcx
    154a:	c7 85 c0 fd ff ff 01 	movl   $0x1,-0x240(%rbp)
    1551:	00 00 00 
    1554:	48 8b bd 00 fe ff ff 	mov    -0x200(%rbp),%rdi
    155b:	48 89 85 a8 fd ff ff 	mov    %rax,-0x258(%rbp)
    1562:	8d 42 65             	lea    0x65(%rdx),%eax
    1565:	48 8b 95 30 fd ff ff 	mov    -0x2d0(%rbp),%rdx
    156c:	89 85 b8 fd ff ff    	mov    %eax,-0x248(%rbp)
    1572:	4a 8d 04 fd 00 00 00 	lea    0x0(,%r15,8),%rax
    1579:	00 
    157a:	48 89 bd a0 fd ff ff 	mov    %rdi,-0x260(%rbp)
    1581:	48 01 c1             	add    %rax,%rcx
    1584:	48 89 8d 70 fd ff ff 	mov    %rcx,-0x290(%rbp)
    158b:	48 8b 8d 38 fd ff ff 	mov    -0x2c8(%rbp),%rcx
    1592:	48 8d 4c 08 08       	lea    0x8(%rax,%rcx,1),%rcx
    1597:	48 89 8d 68 fd ff ff 	mov    %rcx,-0x298(%rbp)
    159e:	48 8d 4c 10 08       	lea    0x8(%rax,%rdx,1),%rcx
    15a3:	48 01 f8             	add    %rdi,%rax
    15a6:	48 89 8d 60 fd ff ff 	mov    %rcx,-0x2a0(%rbp)
    15ad:	48 89 85 58 fd ff ff 	mov    %rax,-0x2a8(%rbp)
    15b4:	31 ff                	xor    %edi,%edi
    15b6:	be ff ff ff ff       	mov    $0xffffffff,%esi
    15bb:	44 8b b5 c0 fd ff ff 	mov    -0x240(%rbp),%r14d
    15c2:	44 89 b5 c4 fd ff ff 	mov    %r14d,-0x23c(%rbp)
    15c9:	e8 22 fb ff ff       	call   10f0 <_gfortran_caf_num_images@plt>
    15ce:	8b 8d 7c fd ff ff    	mov    -0x284(%rbp),%ecx
    15d4:	41 89 c0             	mov    %eax,%r8d
    15d7:	89 d8                	mov    %ebx,%eax
    15d9:	99                   	cltd   
    15da:	41 f7 f8             	idiv   %r8d
    15dd:	39 c8                	cmp    %ecx,%eax
    15df:	0f 4f c1             	cmovg  %ecx,%eax
    15e2:	41 83 c6 64          	add    $0x64,%r14d
    15e6:	89 85 bc fd ff ff    	mov    %eax,-0x244(%rbp)
    15ec:	44 89 b5 c0 fd ff ff 	mov    %r14d,-0x240(%rbp)
    15f3:	3b 85 78 fd ff ff    	cmp    -0x288(%rbp),%eax
    15f9:	0f 8c e1 03 00 00    	jl     19e0 <__acceleratemodule_MOD_accelerateall+0x6f0>
    15ff:	48 8b 85 80 fd ff ff 	mov    -0x280(%rbp),%rax
    1606:	48 ff c0             	inc    %rax
    1609:	48 89 85 d0 fd ff ff 	mov    %rax,-0x230(%rbp)
    1610:	48 8b 85 58 fd ff ff 	mov    -0x2a8(%rbp),%rax
    1617:	48 89 85 c8 fd ff ff 	mov    %rax,-0x238(%rbp)
    161e:	48 8b 85 60 fd ff ff 	mov    -0x2a0(%rbp),%rax
    1625:	48 89 85 68 fe ff ff 	mov    %rax,-0x198(%rbp)
    162c:	48 8b 85 68 fd ff ff 	mov    -0x298(%rbp),%rax
    1633:	48 89 85 60 fe ff ff 	mov    %rax,-0x1a0(%rbp)
    163a:	48 8b 85 70 fd ff ff 	mov    -0x290(%rbp),%rax
    1641:	48 89 85 58 fe ff ff 	mov    %rax,-0x1a8(%rbp)
    1648:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    164f:	00 
    1650:	31 ff                	xor    %edi,%edi
    1652:	be ff ff ff ff       	mov    $0xffffffff,%esi
    1657:	e8 94 fa ff ff       	call   10f0 <_gfortran_caf_num_images@plt>
    165c:	41 89 c0             	mov    %eax,%r8d
    165f:	89 d8                	mov    %ebx,%eax
    1661:	8b 9d c0 fd ff ff    	mov    -0x240(%rbp),%ebx
    1667:	99                   	cltd   
    1668:	41 f7 f8             	idiv   %r8d
    166b:	39 d8                	cmp    %ebx,%eax
    166d:	0f 4f c3             	cmovg  %ebx,%eax
    1670:	89 85 54 fe ff ff    	mov    %eax,-0x1ac(%rbp)
    1676:	39 85 c4 fd ff ff    	cmp    %eax,-0x23c(%rbp)
    167c:	0f 8f d5 02 00 00    	jg     1957 <__acceleratemodule_MOD_accelerateall+0x667>
    1682:	48 8b 85 48 fe ff ff 	mov    -0x1b8(%rbp),%rax
    1689:	48 8d 9d 90 fe ff ff 	lea    -0x170(%rbp),%rbx
    1690:	4c 8d bd 30 ff ff ff 	lea    -0xd0(%rbp),%r15
    1697:	83 bd 50 fe ff ff 01 	cmpl   $0x1,-0x1b0(%rbp)
    169e:	4c 8b b5 a8 fd ff ff 	mov    -0x258(%rbp),%r14
    16a5:	48 0f 44 9d c8 fd ff 	cmove  -0x238(%rbp),%rbx
    16ac:	ff 
    16ad:	4c 8b ad a0 fd ff ff 	mov    -0x260(%rbp),%r13
    16b4:	48 f7 d8             	neg    %rax
    16b7:	48 89 85 40 fe ff ff 	mov    %rax,-0x1c0(%rbp)
    16be:	8b 85 c4 fd ff ff    	mov    -0x23c(%rbp),%eax
    16c4:	89 85 78 fe ff ff    	mov    %eax,-0x188(%rbp)
    16ca:	48 8d 85 b0 fe ff ff 	lea    -0x150(%rbp),%rax
    16d1:	48 89 85 38 fe ff ff 	mov    %rax,-0x1c8(%rbp)
    16d8:	48 8d 45 80          	lea    -0x80(%rbp),%rax
    16dc:	48 89 85 30 fe ff ff 	mov    %rax,-0x1d0(%rbp)
    16e3:	48 8d 85 88 fe ff ff 	lea    -0x178(%rbp),%rax
    16ea:	48 89 85 20 fe ff ff 	mov    %rax,-0x1e0(%rbp)
    16f1:	48 8d 85 d0 fe ff ff 	lea    -0x130(%rbp),%rax
    16f8:	48 89 85 28 fe ff ff 	mov    %rax,-0x1d8(%rbp)
    16ff:	48 8d 85 00 ff ff ff 	lea    -0x100(%rbp),%rax
    1706:	48 89 85 18 fe ff ff 	mov    %rax,-0x1e8(%rbp)
    170d:	0f 1f 00             	nopl   (%rax)
    1710:	83 bd 50 fe ff ff 01 	cmpl   $0x1,-0x1b0(%rbp)
    1717:	0f 85 83 02 00 00    	jne    19a0 <__acceleratemodule_MOD_accelerateall+0x6b0>
    171d:	48 8b 85 48 fe ff ff 	mov    -0x1b8(%rbp),%rax
    1724:	c5 fd 6f 2d b4 1a 00 	vmovdqa 0x1ab4(%rip),%ymm5        # 31e0 <options.103.4+0x20>
    172b:	00 
    172c:	48 c7 45 b0 01 00 00 	movq   $0x1,-0x50(%rbp)
    1733:	00 
    1734:	48 c7 45 b8 03 00 00 	movq   $0x3,-0x48(%rbp)
    173b:	00 
    173c:	4c 89 6d 80          	mov    %r13,-0x80(%rbp)
    1740:	48 c7 45 90 08 00 00 	movq   $0x8,-0x70(%rbp)
    1747:	00 
    1748:	48 c7 45 a0 08 00 00 	movq   $0x8,-0x60(%rbp)
    174f:	00 
    1750:	48 c7 85 38 ff ff ff 	movq   $0x0,-0xc8(%rbp)
    1757:	00 00 00 00 
    175b:	48 c7 85 40 ff ff ff 	movq   $0x8,-0xc0(%rbp)
    1762:	08 00 00 00 
    1766:	48 89 45 a8          	mov    %rax,-0x58(%rbp)
    176a:	48 8b 85 40 fe ff ff 	mov    -0x1c0(%rbp),%rax
    1771:	48 8b 8d 38 fe ff ff 	mov    -0x1c8(%rbp),%rcx
    1778:	48 89 45 88          	mov    %rax,-0x78(%rbp)
    177c:	48 b8 00 00 00 00 01 	movabs $0x30100000000,%rax
    1783:	03 00 00 
    1786:	c5 fd 7f ad 50 ff ff 	vmovdqa %ymm5,-0xb0(%rbp)
    178d:	ff 
    178e:	48 89 8d 30 ff ff ff 	mov    %rcx,-0xd0(%rbp)
    1795:	48 89 45 98          	mov    %rax,-0x68(%rbp)
    1799:	48 89 85 48 ff ff ff 	mov    %rax,-0xb8(%rbp)
    17a0:	48 8b b5 e8 fd ff ff 	mov    -0x218(%rbp),%rsi
    17a7:	48 03 75 80          	add    -0x80(%rbp),%rsi
    17ab:	6a 00                	push   $0x0
    17ad:	4d 89 f9             	mov    %r15,%r9
    17b0:	48 2b b5 00 fe ff ff 	sub    -0x200(%rbp),%rsi
    17b7:	48 8b 8d 30 fe ff ff 	mov    -0x1d0(%rbp),%rcx
    17be:	6a 00                	push   $0x0
    17c0:	45 31 c0             	xor    %r8d,%r8d
    17c3:	48 8b bd f0 fd ff ff 	mov    -0x210(%rbp),%rdi
    17ca:	6a 08                	push   $0x8
    17cc:	44 89 e2             	mov    %r12d,%edx
    17cf:	6a 08                	push   $0x8
    17d1:	c5 f8 77             	vzeroupper 
    17d4:	e8 77 f9 ff ff       	call   1150 <_gfortran_caf_get@plt>
    17d9:	48 83 c4 20          	add    $0x20,%rsp
    17dd:	4c 89 ff             	mov    %r15,%rdi
    17e0:	e8 eb f8 ff ff       	call   10d0 <_gfortran_internal_pack@plt>
    17e5:	48 8b 95 20 fe ff ff 	mov    -0x1e0(%rbp),%rdx
    17ec:	48 c7 85 e0 fe ff ff 	movq   $0x8,-0x120(%rbp)
    17f3:	08 00 00 00 
    17f7:	48 c7 85 10 ff ff ff 	movq   $0x8,-0xf0(%rbp)
    17fe:	08 00 00 00 
    1802:	48 89 85 70 fe ff ff 	mov    %rax,-0x190(%rbp)
    1809:	48 b8 00 00 00 00 00 	movabs $0x30000000000,%rax
    1810:	03 00 00 
    1813:	4c 89 b5 00 ff ff ff 	mov    %r14,-0x100(%rbp)
    181a:	48 89 85 e8 fe ff ff 	mov    %rax,-0x118(%rbp)
    1821:	48 89 85 18 ff ff ff 	mov    %rax,-0xe8(%rbp)
    1828:	48 89 95 d0 fe ff ff 	mov    %rdx,-0x130(%rbp)
    182f:	48 8b b5 d8 fd ff ff 	mov    -0x228(%rbp),%rsi
    1836:	48 03 b5 00 ff ff ff 	add    -0x100(%rbp),%rsi
    183d:	6a 00                	push   $0x0
    183f:	45 31 c0             	xor    %r8d,%r8d
    1842:	48 2b b5 f8 fd ff ff 	sub    -0x208(%rbp),%rsi
    1849:	4c 8b 8d 28 fe ff ff 	mov    -0x1d8(%rbp),%r9
    1850:	6a 00                	push   $0x0
    1852:	44 89 e2             	mov    %r12d,%edx
    1855:	48 8b 8d 18 fe ff ff 	mov    -0x1e8(%rbp),%rcx
    185c:	48 8b bd e0 fd ff ff 	mov    -0x220(%rbp),%rdi
    1863:	6a 08                	push   $0x8
    1865:	6a 08                	push   $0x8
    1867:	e8 e4 f8 ff ff       	call   1150 <_gfortran_caf_get@plt>
    186c:	48 8b 85 70 fe ff ff 	mov    -0x190(%rbp),%rax
    1873:	48 83 c4 20          	add    $0x20,%rsp
    1877:	c5 fb 10 35 81 19 00 	vmovsd 0x1981(%rip),%xmm6        # 3200 <options.103.4+0x40>
    187e:	00 
    187f:	c5 fb 10 50 08       	vmovsd 0x8(%rax),%xmm2
    1884:	c5 fb 10 18          	vmovsd (%rax),%xmm3
    1888:	c5 eb 5c 53 08       	vsubsd 0x8(%rbx),%xmm2,%xmm2
    188d:	c5 e3 5c 1b          	vsubsd (%rbx),%xmm3,%xmm3
    1891:	c5 fb 10 48 10       	vmovsd 0x10(%rax),%xmm1
    1896:	c5 f3 5c 4b 10       	vsubsd 0x10(%rbx),%xmm1,%xmm1
    189b:	c5 eb 59 c2          	vmulsd %xmm2,%xmm2,%xmm0
    189f:	c4 e2 e1 b9 c3       	vfmadd231sd %xmm3,%xmm3,%xmm0
    18a4:	c4 e2 f1 b9 c1       	vfmadd231sd %xmm1,%xmm1,%xmm0
    18a9:	c5 fb 51 e0          	vsqrtsd %xmm0,%xmm0,%xmm4
    18ad:	c5 fb 59 c4          	vmulsd %xmm4,%xmm0,%xmm0
    18b1:	c5 cb 5e c0          	vdivsd %xmm0,%xmm6,%xmm0
    18b5:	c5 fb 59 85 88 fe ff 	vmulsd -0x178(%rbp),%xmm0,%xmm0
    18bc:	ff 
    18bd:	c5 e3 59 d8          	vmulsd %xmm0,%xmm3,%xmm3
    18c1:	c5 eb 59 d0          	vmulsd %xmm0,%xmm2,%xmm2
    18c5:	c5 f3 59 c0          	vmulsd %xmm0,%xmm1,%xmm0
    18c9:	48 39 85 30 ff ff ff 	cmp    %rax,-0xd0(%rbp)
    18d0:	74 38                	je     190a <__acceleratemodule_MOD_accelerateall+0x61a>
    18d2:	48 89 c7             	mov    %rax,%rdi
    18d5:	c5 fb 11 85 08 fe ff 	vmovsd %xmm0,-0x1f8(%rbp)
    18dc:	ff 
    18dd:	c5 fb 11 95 10 fe ff 	vmovsd %xmm2,-0x1f0(%rbp)
    18e4:	ff 
    18e5:	c5 fb 11 9d 70 fe ff 	vmovsd %xmm3,-0x190(%rbp)
    18ec:	ff 
    18ed:	e8 7e f7 ff ff       	call   1070 <free@plt>
    18f2:	c5 fb 10 85 08 fe ff 	vmovsd -0x1f8(%rbp),%xmm0
    18f9:	ff 
    18fa:	c5 fb 10 95 10 fe ff 	vmovsd -0x1f0(%rbp),%xmm2
    1901:	ff 
    1902:	c5 fb 10 9d 70 fe ff 	vmovsd -0x190(%rbp),%xmm3
    1909:	ff 
    190a:	48 8b 85 58 fe ff ff 	mov    -0x1a8(%rbp),%rax
    1911:	ff 85 78 fe ff ff    	incl   -0x188(%rbp)
    1917:	49 83 c5 08          	add    $0x8,%r13
    191b:	49 83 c6 08          	add    $0x8,%r14
    191f:	c5 e3 58 18          	vaddsd (%rax),%xmm3,%xmm3
    1923:	c5 fb 11 18          	vmovsd %xmm3,(%rax)
    1927:	48 8b 85 60 fe ff ff 	mov    -0x1a0(%rbp),%rax
    192e:	c5 eb 58 10          	vaddsd (%rax),%xmm2,%xmm2
    1932:	c5 fb 11 10          	vmovsd %xmm2,(%rax)
    1936:	48 8b 85 68 fe ff ff 	mov    -0x198(%rbp),%rax
    193d:	c5 fb 58 00          	vaddsd (%rax),%xmm0,%xmm0
    1941:	c5 fb 11 00          	vmovsd %xmm0,(%rax)
    1945:	8b 85 78 fe ff ff    	mov    -0x188(%rbp),%eax
    194b:	39 85 54 fe ff ff    	cmp    %eax,-0x1ac(%rbp)
    1951:	0f 8d b9 fd ff ff    	jge    1710 <__acceleratemodule_MOD_accelerateall+0x420>
    1957:	48 ff 85 d0 fd ff ff 	incq   -0x230(%rbp)
    195e:	48 83 85 58 fe ff ff 	addq   $0x8,-0x1a8(%rbp)
    1965:	08 
    1966:	48 83 85 60 fe ff ff 	addq   $0x8,-0x1a0(%rbp)
    196d:	08 
    196e:	48 83 85 68 fe ff ff 	addq   $0x8,-0x198(%rbp)
    1975:	08 
    1976:	48 83 85 c8 fd ff ff 	addq   $0x8,-0x238(%rbp)
    197d:	08 
    197e:	48 8b 85 d0 fd ff ff 	mov    -0x230(%rbp),%rax
    1985:	39 85 bc fd ff ff    	cmp    %eax,-0x244(%rbp)
    198b:	7c 53                	jl     19e0 <__acceleratemodule_MOD_accelerateall+0x6f0>
    198d:	48 8b 85 b0 fd ff ff 	mov    -0x250(%rbp),%rax
    1994:	8b 18                	mov    (%rax),%ebx
    1996:	e9 b5 fc ff ff       	jmp    1650 <__acceleratemodule_MOD_accelerateall+0x360>
    199b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
    19a0:	48 8b 85 d0 fd ff ff 	mov    -0x230(%rbp),%rax
    19a7:	48 8b 95 98 fd ff ff 	mov    -0x268(%rbp),%rdx
    19ae:	48 8b 8d 90 fd ff ff 	mov    -0x270(%rbp),%rcx
    19b5:	c5 fb 10 04 c2       	vmovsd (%rdx,%rax,8),%xmm0
    19ba:	48 8b 95 88 fd ff ff 	mov    -0x278(%rbp),%rdx
    19c1:	c5 f9 16 04 c1       	vmovhpd (%rcx,%rax,8),%xmm0,%xmm0
    19c6:	c5 f9 29 85 90 fe ff 	vmovapd %xmm0,-0x170(%rbp)
    19cd:	ff 
    19ce:	c5 fb 10 04 c2       	vmovsd (%rdx,%rax,8),%xmm0
    19d3:	c5 fb 11 85 a0 fe ff 	vmovsd %xmm0,-0x160(%rbp)
    19da:	ff 
    19db:	e9 3d fd ff ff       	jmp    171d <__acceleratemodule_MOD_accelerateall+0x42d>
    19e0:	48 81 85 a0 fd ff ff 	addq   $0x320,-0x260(%rbp)
    19e7:	20 03 00 00 
    19eb:	48 81 85 a8 fd ff ff 	addq   $0x320,-0x258(%rbp)
    19f2:	20 03 00 00 
    19f6:	8b 9d c0 fd ff ff    	mov    -0x240(%rbp),%ebx
    19fc:	39 9d b8 fd ff ff    	cmp    %ebx,-0x248(%rbp)
    1a02:	74 0e                	je     1a12 <__acceleratemodule_MOD_accelerateall+0x722>
    1a04:	48 8b 85 b0 fd ff ff 	mov    -0x250(%rbp),%rax
    1a0b:	8b 18                	mov    (%rax),%ebx
    1a0d:	e9 a2 fb ff ff       	jmp    15b4 <__acceleratemodule_MOD_accelerateall+0x2c4>
    1a12:	48 83 85 80 fd ff ff 	addq   $0x64,-0x280(%rbp)
    1a19:	64 
    1a1a:	48 8b 85 80 fd ff ff 	mov    -0x280(%rbp),%rax
    1a21:	48 3b 85 40 fd ff ff 	cmp    -0x2c0(%rbp),%rax
    1a28:	74 0e                	je     1a38 <__acceleratemodule_MOD_accelerateall+0x748>
    1a2a:	48 8b 85 b0 fd ff ff 	mov    -0x250(%rbp),%rax
    1a31:	8b 18                	mov    (%rax),%ebx
    1a33:	e9 b8 fa ff ff       	jmp    14f0 <__acceleratemodule_MOD_accelerateall+0x200>
    1a38:	41 ff c4             	inc    %r12d
    1a3b:	44 39 a5 4c fd ff ff 	cmp    %r12d,-0x2b4(%rbp)
    1a42:	7c 0e                	jl     1a52 <__acceleratemodule_MOD_accelerateall+0x762>
    1a44:	48 8b 85 b0 fd ff ff 	mov    -0x250(%rbp),%rax
    1a4b:	8b 18                	mov    (%rax),%ebx
    1a4d:	e9 5a fa ff ff       	jmp    14ac <__acceleratemodule_MOD_accelerateall+0x1bc>
    1a52:	48 8d 65 d0          	lea    -0x30(%rbp),%rsp
    1a56:	5b                   	pop    %rbx
    1a57:	41 5a                	pop    %r10
    1a59:	41 5c                	pop    %r12
    1a5b:	41 5d                	pop    %r13
    1a5d:	41 5e                	pop    %r14
    1a5f:	41 5f                	pop    %r15
    1a61:	5d                   	pop    %rbp
    1a62:	49 8d 62 f8          	lea    -0x8(%r10),%rsp
    1a66:	c3                   	ret    
    1a67:	66 0f 1f 84 00 00 00 	nopw   0x0(%rax,%rax,1)
    1a6e:	00 00 

0000000000001a70 <__acceleratemodule_MOD_accelerate>:
    1a70:	48 89 f0             	mov    %rsi,%rax
    1a73:	c5 fb 10 4a 08       	vmovsd 0x8(%rdx),%xmm1
    1a78:	c5 fb 10 02          	vmovsd (%rdx),%xmm0
    1a7c:	49 89 c8             	mov    %rcx,%r8
    1a7f:	c5 f3 5c 48 08       	vsubsd 0x8(%rax),%xmm1,%xmm1
    1a84:	c5 fb 5c 00          	vsubsd (%rax),%xmm0,%xmm0
    1a88:	b9 01 00 00 00       	mov    $0x1,%ecx
    1a8d:	c5 fb 10 52 10       	vmovsd 0x10(%rdx),%xmm2
    1a92:	48 8b 77 28          	mov    0x28(%rdi),%rsi
    1a96:	c5 eb 5c 50 10       	vsubsd 0x10(%rax),%xmm2,%xmm2
    1a9b:	48 85 f6             	test   %rsi,%rsi
    1a9e:	48 0f 44 f1          	cmove  %rcx,%rsi
    1aa2:	48 8b 0f             	mov    (%rdi),%rcx
    1aa5:	c5 f3 59 c9          	vmulsd %xmm1,%xmm1,%xmm1
    1aa9:	c4 e2 f1 99 c0       	vfmadd132sd %xmm0,%xmm1,%xmm0
    1aae:	c4 e2 e9 b9 c2       	vfmadd231sd %xmm2,%xmm2,%xmm0
    1ab3:	c5 fb 51 c8          	vsqrtsd %xmm0,%xmm0,%xmm1
    1ab7:	c5 f3 59 c8          	vmulsd %xmm0,%xmm1,%xmm1
    1abb:	c4 c1 7b 10 00       	vmovsd (%r8),%xmm0
    1ac0:	c5 fb 5e c1          	vdivsd %xmm1,%xmm0,%xmm0
    1ac4:	c5 f9 10 0a          	vmovupd (%rdx),%xmm1
    1ac8:	c5 f1 5c 08          	vsubpd (%rax),%xmm1,%xmm1
    1acc:	c5 fb 12 d8          	vmovddup %xmm0,%xmm3
    1ad0:	c5 f1 59 cb          	vmulpd %xmm3,%xmm1,%xmm1
    1ad4:	48 83 fe 01          	cmp    $0x1,%rsi
    1ad8:	75 16                	jne    1af0 <__acceleratemodule_MOD_accelerate+0x80>
    1ada:	c5 fb 59 c2          	vmulsd %xmm2,%xmm0,%xmm0
    1ade:	c5 f9 11 09          	vmovupd %xmm1,(%rcx)
    1ae2:	c5 fb 11 41 10       	vmovsd %xmm0,0x10(%rcx)
    1ae7:	c3                   	ret    
    1ae8:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    1aef:	00 
    1af0:	c5 fb 59 c2          	vmulsd %xmm2,%xmm0,%xmm0
    1af4:	c5 f9 13 09          	vmovlpd %xmm1,(%rcx)
    1af8:	c5 f9 17 0c f1       	vmovhpd %xmm1,(%rcx,%rsi,8)
    1afd:	48 01 f6             	add    %rsi,%rsi
    1b00:	c5 fb 11 04 f1       	vmovsd %xmm0,(%rcx,%rsi,8)
    1b05:	c3                   	ret    
    1b06:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    1b0d:	00 00 00 

0000000000001b10 <__nbodymodule_MOD_advanceit>:
    1b10:	41 55                	push   %r13
    1b12:	4c 8d 6c 24 10       	lea    0x10(%rsp),%r13
    1b17:	48 83 e4 e0          	and    $0xffffffffffffffe0,%rsp
    1b1b:	41 ff 75 f8          	push   -0x8(%r13)
    1b1f:	55                   	push   %rbp
    1b20:	48 89 e5             	mov    %rsp,%rbp
    1b23:	41 57                	push   %r15
    1b25:	41 56                	push   %r14
    1b27:	41 55                	push   %r13
    1b29:	41 54                	push   %r12
    1b2b:	53                   	push   %rbx
    1b2c:	49 89 fe             	mov    %rdi,%r14
    1b2f:	48 89 f3             	mov    %rsi,%rbx
    1b32:	48 83 ec 68          	sub    $0x68,%rsp
    1b36:	49 8b 75 20          	mov    0x20(%r13),%rsi
    1b3a:	49 8b 7d 28          	mov    0x28(%r13),%rdi
    1b3e:	48 89 95 78 ff ff ff 	mov    %rdx,-0x88(%rbp)
    1b45:	49 8b 45 00          	mov    0x0(%r13),%rax
    1b49:	49 89 cf             	mov    %rcx,%r15
    1b4c:	4c 89 45 b8          	mov    %r8,-0x48(%rbp)
    1b50:	4c 89 c9             	mov    %r9,%rcx
    1b53:	4d 8b 45 30          	mov    0x30(%r13),%r8
    1b57:	48 89 4d 90          	mov    %rcx,-0x70(%rbp)
    1b5b:	4d 8b 4d 38          	mov    0x38(%r13),%r9
    1b5f:	49 8b 55 08          	mov    0x8(%r13),%rdx
    1b63:	44 8b 21             	mov    (%rcx),%r12d
    1b66:	48 89 75 a0          	mov    %rsi,-0x60(%rbp)
    1b6a:	48 89 7d 98          	mov    %rdi,-0x68(%rbp)
    1b6e:	be ff ff ff ff       	mov    $0xffffffff,%esi
    1b73:	31 ff                	xor    %edi,%edi
    1b75:	48 89 45 b0          	mov    %rax,-0x50(%rbp)
    1b79:	4c 89 45 80          	mov    %r8,-0x80(%rbp)
    1b7d:	4c 89 4d 88          	mov    %r9,-0x78(%rbp)
    1b81:	48 89 55 a8          	mov    %rdx,-0x58(%rbp)
    1b85:	e8 66 f5 ff ff       	call   10f0 <_gfortran_caf_num_images@plt>
    1b8a:	be ff ff ff ff       	mov    $0xffffffff,%esi
    1b8f:	41 89 c3             	mov    %eax,%r11d
    1b92:	44 89 e0             	mov    %r12d,%eax
    1b95:	99                   	cltd   
    1b96:	41 f7 fb             	idiv   %r11d
    1b99:	4c 63 e8             	movslq %eax,%r13
    1b9c:	31 c0                	xor    %eax,%eax
    1b9e:	4d 85 ed             	test   %r13,%r13
    1ba1:	4c 0f 48 e8          	cmovs  %rax,%r13
    1ba5:	31 ff                	xor    %edi,%edi
    1ba7:	e8 44 f5 ff ff       	call   10f0 <_gfortran_caf_num_images@plt>
    1bac:	41 89 c3             	mov    %eax,%r11d
    1baf:	44 89 e0             	mov    %r12d,%eax
    1bb2:	99                   	cltd   
    1bb3:	41 f7 fb             	idiv   %r11d
    1bb6:	48 63 f0             	movslq %eax,%rsi
    1bb9:	31 c0                	xor    %eax,%eax
    1bbb:	48 85 f6             	test   %rsi,%rsi
    1bbe:	48 0f 49 c6          	cmovns %rsi,%rax
    1bc2:	31 ff                	xor    %edi,%edi
    1bc4:	be ff ff ff ff       	mov    $0xffffffff,%esi
    1bc9:	48 89 45 c0          	mov    %rax,-0x40(%rbp)
    1bcd:	e8 1e f5 ff ff       	call   10f0 <_gfortran_caf_num_images@plt>
    1bd2:	31 ff                	xor    %edi,%edi
    1bd4:	be ff ff ff ff       	mov    $0xffffffff,%esi
    1bd9:	e8 12 f5 ff ff       	call   10f0 <_gfortran_caf_num_images@plt>
    1bde:	4c 89 f6             	mov    %r14,%rsi
    1be1:	4c 89 ff             	mov    %r15,%rdi
    1be4:	41 89 c3             	mov    %eax,%r11d
    1be7:	44 89 e0             	mov    %r12d,%eax
    1bea:	ff 75 98             	push   -0x68(%rbp)
    1bed:	4c 8b 4d 88          	mov    -0x78(%rbp),%r9
    1bf1:	99                   	cltd   
    1bf2:	ff 75 a0             	push   -0x60(%rbp)
    1bf5:	41 f7 fb             	idiv   %r11d
    1bf8:	ff 75 a8             	push   -0x58(%rbp)
    1bfb:	4c 8b 45 80          	mov    -0x80(%rbp),%r8
    1bff:	ff 75 b0             	push   -0x50(%rbp)
    1c02:	48 8b 4d 90          	mov    -0x70(%rbp),%rcx
    1c06:	48 8b 95 78 ff ff ff 	mov    -0x88(%rbp),%rdx
    1c0d:	4c 63 e0             	movslq %eax,%r12
    1c10:	31 c0                	xor    %eax,%eax
    1c12:	4d 85 e4             	test   %r12,%r12
    1c15:	49 0f 49 c4          	cmovns %r12,%rax
    1c19:	48 89 45 c8          	mov    %rax,-0x38(%rbp)
    1c1d:	e8 ce f6 ff ff       	call   12f0 <__acceleratemodule_MOD_accelerateall>
    1c22:	48 8b 45 b8          	mov    -0x48(%rbp),%rax
    1c26:	48 83 c4 20          	add    $0x20,%rsp
    1c2a:	c5 fb 10 00          	vmovsd (%rax),%xmm0
    1c2e:	4d 85 e4             	test   %r12,%r12
    1c31:	0f 8e e0 01 00 00    	jle    1e17 <__nbodymodule_MOD_advanceit+0x307>
    1c37:	4d 89 e3             	mov    %r12,%r11
    1c3a:	49 8d 44 24 ff       	lea    -0x1(%r12),%rax
    1c3f:	4c 89 e6             	mov    %r12,%rsi
    1c42:	48 c7 c1 ff ff ff ff 	mov    $0xffffffffffffffff,%rcx
    1c49:	49 83 e3 fc          	and    $0xfffffffffffffffc,%r11
    1c4d:	48 89 45 b8          	mov    %rax,-0x48(%rbp)
    1c51:	48 c1 ee 02          	shr    $0x2,%rsi
    1c55:	48 c7 c2 ff ff ff ff 	mov    $0xffffffffffffffff,%rdx
    1c5c:	49 8d 43 01          	lea    0x1(%r11),%rax
    1c60:	48 c1 e6 05          	shl    $0x5,%rsi
    1c64:	41 ba 03 00 00 00    	mov    $0x3,%r10d
    1c6a:	c5 fb 12 d8          	vmovddup %xmm0,%xmm3
    1c6e:	48 89 45 a0          	mov    %rax,-0x60(%rbp)
    1c72:	48 8d 43 08          	lea    0x8(%rbx),%rax
    1c76:	c4 e2 7d 19 d0       	vbroadcastsd %xmm0,%ymm2
    1c7b:	48 89 45 b0          	mov    %rax,-0x50(%rbp)
    1c7f:	49 8d 47 08          	lea    0x8(%r15),%rax
    1c83:	48 89 45 a8          	mov    %rax,-0x58(%rbp)
    1c87:	48 83 7d b8 02       	cmpq   $0x2,-0x48(%rbp)
    1c8c:	0f 86 a4 01 00 00    	jbe    1e36 <__nbodymodule_MOD_advanceit+0x326>
    1c92:	48 8b 45 b0          	mov    -0x50(%rbp),%rax
    1c96:	48 8d 3c d0          	lea    (%rax,%rdx,8),%rdi
    1c9a:	48 8b 45 a8          	mov    -0x58(%rbp),%rax
    1c9e:	4c 8d 04 c8          	lea    (%rax,%rcx,8),%r8
    1ca2:	31 c0                	xor    %eax,%eax
    1ca4:	66 66 2e 0f 1f 84 00 	data16 cs nopw 0x0(%rax,%rax,1)
    1cab:	00 00 00 00 
    1caf:	90                   	nop
    1cb0:	c4 c1 7d 10 0c 00    	vmovupd (%r8,%rax,1),%ymm1
    1cb6:	c4 e2 ed a8 0c 07    	vfmadd213pd (%rdi,%rax,1),%ymm2,%ymm1
    1cbc:	c5 fd 11 0c 07       	vmovupd %ymm1,(%rdi,%rax,1)
    1cc1:	48 83 c0 20          	add    $0x20,%rax
    1cc5:	48 39 f0             	cmp    %rsi,%rax
    1cc8:	75 e6                	jne    1cb0 <__nbodymodule_MOD_advanceit+0x1a0>
    1cca:	4d 39 dc             	cmp    %r11,%r12
    1ccd:	74 58                	je     1d27 <__nbodymodule_MOD_advanceit+0x217>
    1ccf:	48 8b 45 a0          	mov    -0x60(%rbp),%rax
    1cd3:	4d 89 d8             	mov    %r11,%r8
    1cd6:	4c 89 e7             	mov    %r12,%rdi
    1cd9:	4c 29 c7             	sub    %r8,%rdi
    1cdc:	48 83 ff 01          	cmp    $0x1,%rdi
    1ce0:	74 2d                	je     1d0f <__nbodymodule_MOD_advanceit+0x1ff>
    1ce2:	4d 8d 4c 10 01       	lea    0x1(%r8,%rdx,1),%r9
    1ce7:	4d 8d 44 08 01       	lea    0x1(%r8,%rcx,1),%r8
    1cec:	c4 81 79 10 0c c7    	vmovupd (%r15,%r8,8),%xmm1
    1cf2:	4e 8d 0c cb          	lea    (%rbx,%r9,8),%r9
    1cf6:	49 89 f8             	mov    %rdi,%r8
    1cf9:	c4 c2 e1 a8 09       	vfmadd213pd (%r9),%xmm3,%xmm1
    1cfe:	49 83 e0 fe          	and    $0xfffffffffffffffe,%r8
    1d02:	4c 01 c0             	add    %r8,%rax
    1d05:	c4 c1 79 11 09       	vmovupd %xmm1,(%r9)
    1d0a:	4c 39 c7             	cmp    %r8,%rdi
    1d0d:	74 18                	je     1d27 <__nbodymodule_MOD_advanceit+0x217>
    1d0f:	48 8d 3c 02          	lea    (%rdx,%rax,1),%rdi
    1d13:	48 01 c8             	add    %rcx,%rax
    1d16:	c4 c1 7b 10 0c c7    	vmovsd (%r15,%rax,8),%xmm1
    1d1c:	c4 e2 f9 a9 0c fb    	vfmadd213sd (%rbx,%rdi,8),%xmm0,%xmm1
    1d22:	c5 fb 11 0c fb       	vmovsd %xmm1,(%rbx,%rdi,8)
    1d27:	48 03 55 c8          	add    -0x38(%rbp),%rdx
    1d2b:	4c 01 e9             	add    %r13,%rcx
    1d2e:	49 ff ca             	dec    %r10
    1d31:	0f 85 50 ff ff ff    	jne    1c87 <__nbodymodule_MOD_advanceit+0x177>
    1d37:	4d 89 e3             	mov    %r12,%r11
    1d3a:	49 8d 44 24 ff       	lea    -0x1(%r12),%rax
    1d3f:	4c 89 e6             	mov    %r12,%rsi
    1d42:	48 c7 c1 ff ff ff ff 	mov    $0xffffffffffffffff,%rcx
    1d49:	49 83 e3 fc          	and    $0xfffffffffffffffc,%r11
    1d4d:	48 89 45 b8          	mov    %rax,-0x48(%rbp)
    1d51:	48 c1 ee 02          	shr    $0x2,%rsi
    1d55:	48 c7 c2 ff ff ff ff 	mov    $0xffffffffffffffff,%rdx
    1d5c:	49 8d 43 01          	lea    0x1(%r11),%rax
    1d60:	48 c1 e6 05          	shl    $0x5,%rsi
    1d64:	41 b9 03 00 00 00    	mov    $0x3,%r9d
    1d6a:	c5 fb 12 d8          	vmovddup %xmm0,%xmm3
    1d6e:	48 89 45 b0          	mov    %rax,-0x50(%rbp)
    1d72:	4c 8d 7b 08          	lea    0x8(%rbx),%r15
    1d76:	c4 e2 7d 19 d0       	vbroadcastsd %xmm0,%ymm2
    1d7b:	4d 8d 6e 08          	lea    0x8(%r14),%r13
    1d7f:	48 83 7d b8 02       	cmpq   $0x2,-0x48(%rbp)
    1d84:	0f 86 b9 00 00 00    	jbe    1e43 <__nbodymodule_MOD_advanceit+0x333>
    1d8a:	4d 8d 04 d7          	lea    (%r15,%rdx,8),%r8
    1d8e:	49 8d 7c cd 00       	lea    0x0(%r13,%rcx,8),%rdi
    1d93:	31 c0                	xor    %eax,%eax
    1d95:	66 66 2e 0f 1f 84 00 	data16 cs nopw 0x0(%rax,%rax,1)
    1d9c:	00 00 00 00 
    1da0:	c4 c1 6d 59 0c 00    	vmulpd (%r8,%rax,1),%ymm2,%ymm1
    1da6:	c5 fd 11 0c 07       	vmovupd %ymm1,(%rdi,%rax,1)
    1dab:	48 83 c0 20          	add    $0x20,%rax
    1daf:	48 39 c6             	cmp    %rax,%rsi
    1db2:	75 ec                	jne    1da0 <__nbodymodule_MOD_advanceit+0x290>
    1db4:	4d 39 e3             	cmp    %r12,%r11
    1db7:	74 4a                	je     1e03 <__nbodymodule_MOD_advanceit+0x2f3>
    1db9:	48 8b 45 b0          	mov    -0x50(%rbp),%rax
    1dbd:	4d 89 d8             	mov    %r11,%r8
    1dc0:	4c 89 e7             	mov    %r12,%rdi
    1dc3:	4c 29 c7             	sub    %r8,%rdi
    1dc6:	48 83 ff 01          	cmp    $0x1,%rdi
    1dca:	74 25                	je     1df1 <__nbodymodule_MOD_advanceit+0x2e1>
    1dcc:	4d 8d 54 10 01       	lea    0x1(%r8,%rdx,1),%r10
    1dd1:	4d 8d 44 08 01       	lea    0x1(%r8,%rcx,1),%r8
    1dd6:	c4 a1 61 59 0c d3    	vmulpd (%rbx,%r10,8),%xmm3,%xmm1
    1ddc:	c4 81 79 11 0c c6    	vmovupd %xmm1,(%r14,%r8,8)
    1de2:	49 89 f8             	mov    %rdi,%r8
    1de5:	49 83 e0 fe          	and    $0xfffffffffffffffe,%r8
    1de9:	4c 01 c0             	add    %r8,%rax
    1dec:	4c 39 c7             	cmp    %r8,%rdi
    1def:	74 12                	je     1e03 <__nbodymodule_MOD_advanceit+0x2f3>
    1df1:	48 8d 3c 01          	lea    (%rcx,%rax,1),%rdi
    1df5:	48 01 d0             	add    %rdx,%rax
    1df8:	c5 fb 59 0c c3       	vmulsd (%rbx,%rax,8),%xmm0,%xmm1
    1dfd:	c4 c1 7b 11 0c fe    	vmovsd %xmm1,(%r14,%rdi,8)
    1e03:	48 03 55 c8          	add    -0x38(%rbp),%rdx
    1e07:	48 03 4d c0          	add    -0x40(%rbp),%rcx
    1e0b:	49 ff c9             	dec    %r9
    1e0e:	0f 85 6b ff ff ff    	jne    1d7f <__nbodymodule_MOD_advanceit+0x26f>
    1e14:	c5 f8 77             	vzeroupper 
    1e17:	48 8d 65 d8          	lea    -0x28(%rbp),%rsp
    1e1b:	31 d2                	xor    %edx,%edx
    1e1d:	31 f6                	xor    %esi,%esi
    1e1f:	31 ff                	xor    %edi,%edi
    1e21:	5b                   	pop    %rbx
    1e22:	41 5c                	pop    %r12
    1e24:	41 5d                	pop    %r13
    1e26:	41 5e                	pop    %r14
    1e28:	41 5f                	pop    %r15
    1e2a:	5d                   	pop    %rbp
    1e2b:	49 8d 65 f0          	lea    -0x10(%r13),%rsp
    1e2f:	41 5d                	pop    %r13
    1e31:	e9 7a f2 ff ff       	jmp    10b0 <_gfortran_caf_sync_all@plt>
    1e36:	45 31 c0             	xor    %r8d,%r8d
    1e39:	b8 01 00 00 00       	mov    $0x1,%eax
    1e3e:	e9 93 fe ff ff       	jmp    1cd6 <__nbodymodule_MOD_advanceit+0x1c6>
    1e43:	45 31 c0             	xor    %r8d,%r8d
    1e46:	b8 01 00 00 00       	mov    $0x1,%eax
    1e4b:	e9 70 ff ff ff       	jmp    1dc0 <__nbodymodule_MOD_advanceit+0x2b0>

0000000000001e50 <MAIN__>:
    1e50:	55                   	push   %rbp
    1e51:	48 b8 00 00 00 00 02 	movabs $0x30200000000,%rax
    1e58:	03 00 00 
    1e5b:	bf 18 00 00 00       	mov    $0x18,%edi
    1e60:	48 89 e5             	mov    %rsp,%rbp
    1e63:	41 57                	push   %r15
    1e65:	41 56                	push   %r14
    1e67:	41 55                	push   %r13
    1e69:	41 54                	push   %r12
    1e6b:	41 52                	push   %r10
    1e6d:	53                   	push   %rbx
    1e6e:	48 bb 00 00 00 00 01 	movabs $0x30100000000,%rbx
    1e75:	03 00 00 
    1e78:	48 81 ec 50 02 00 00 	sub    $0x250,%rsp
    1e7f:	48 c7 05 16 33 00 00 	movq   $0x0,0x3316(%rip)        # 51a0 <accel.3>
    1e86:	00 00 00 00 
    1e8a:	48 c7 05 7b 33 00 00 	movq   $0x0,0x337b(%rip)        # 5210 <accel.3+0x70>
    1e91:	00 00 00 00 
    1e95:	48 c7 05 10 33 00 00 	movq   $0x8,0x3310(%rip)        # 51b0 <accel.3+0x10>
    1e9c:	08 00 00 00 
    1ea0:	48 89 05 11 33 00 00 	mov    %rax,0x3311(%rip)        # 51b8 <accel.3+0x18>
    1ea7:	48 c7 05 8e 32 00 00 	movq   $0x0,0x328e(%rip)        # 5140 <masses.2>
    1eae:	00 00 00 00 
    1eb2:	48 c7 05 db 32 00 00 	movq   $0x0,0x32db(%rip)        # 5198 <masses.2+0x58>
    1eb9:	00 00 00 00 
    1ebd:	48 c7 05 88 32 00 00 	movq   $0x8,0x3288(%rip)        # 5150 <masses.2+0x10>
    1ec4:	08 00 00 00 
    1ec8:	48 89 1d 89 32 00 00 	mov    %rbx,0x3289(%rip)        # 5158 <masses.2+0x18>
    1ecf:	48 c7 05 e6 31 00 00 	movq   $0x0,0x31e6(%rip)        # 50c0 <positions.1>
    1ed6:	00 00 00 00 
    1eda:	48 c7 05 4b 32 00 00 	movq   $0x0,0x324b(%rip)        # 5130 <positions.1+0x70>
    1ee1:	00 00 00 00 
    1ee5:	48 c7 05 e0 31 00 00 	movq   $0x8,0x31e0(%rip)        # 50d0 <positions.1+0x10>
    1eec:	08 00 00 00 
    1ef0:	48 89 05 e1 31 00 00 	mov    %rax,0x31e1(%rip)        # 50d8 <positions.1+0x18>
    1ef7:	48 c7 05 3e 31 00 00 	movq   $0x0,0x313e(%rip)        # 5040 <velocities.0>
    1efe:	00 00 00 00 
    1f02:	48 c7 05 a3 31 00 00 	movq   $0x0,0x31a3(%rip)        # 50b0 <velocities.0+0x70>
    1f09:	00 00 00 00 
    1f0d:	48 c7 05 38 31 00 00 	movq   $0x8,0x3138(%rip)        # 5050 <velocities.0+0x10>
    1f14:	08 00 00 00 
    1f18:	48 89 05 39 31 00 00 	mov    %rax,0x3139(%rip)        # 5058 <velocities.0+0x18>
    1f1f:	e8 3c f1 ff ff       	call   1060 <malloc@plt>
    1f24:	48 85 c0             	test   %rax,%rax
    1f27:	0f 84 86 08 00 00    	je     27b3 <MAIN__+0x963>
    1f2d:	41 b8 0c 00 00 00    	mov    $0xc,%r8d
    1f33:	31 c9                	xor    %ecx,%ecx
    1f35:	48 89 c6             	mov    %rax,%rsi
    1f38:	49 89 c4             	mov    %rax,%r12
    1f3b:	31 d2                	xor    %edx,%edx
    1f3d:	31 c0                	xor    %eax,%eax
    1f3f:	48 8d 3d 6a 12 00 00 	lea    0x126a(%rip),%rdi        # 31b0 <_IO_stdin_used+0x1b0>
    1f46:	48 8d 9d c0 fd ff ff 	lea    -0x240(%rbp),%rbx
    1f4d:	e8 ce f1 ff ff       	call   1120 <_gfortran_get_command_argument_i4@plt>
    1f52:	4c 8b 3d c7 12 00 00 	mov    0x12c7(%rip),%r15        # 3220 <options.103.4+0x60>
    1f59:	4c 8d 35 bf 10 00 00 	lea    0x10bf(%rip),%r14        # 301f <_IO_stdin_used+0x1f>
    1f60:	48 89 df             	mov    %rbx,%rdi
    1f63:	4c 8d ad bc fd ff ff 	lea    -0x244(%rbp),%r13
    1f6a:	4c 89 a5 30 fe ff ff 	mov    %r12,-0x1d0(%rbp)
    1f71:	4c 89 b5 c8 fd ff ff 	mov    %r14,-0x238(%rbp)
    1f78:	c7 85 d0 fd ff ff 5d 	movl   $0x5d,-0x230(%rbp)
    1f7f:	00 00 00 
    1f82:	48 c7 85 38 fe ff ff 	movq   $0xc,-0x1c8(%rbp)
    1f89:	0c 00 00 00 
    1f8d:	48 c7 85 08 fe ff ff 	movq   $0x0,-0x1f8(%rbp)
    1f94:	00 00 00 00 
    1f98:	49 83 c4 0c          	add    $0xc,%r12
    1f9c:	4c 89 bd c0 fd ff ff 	mov    %r15,-0x240(%rbp)
    1fa3:	e8 18 f1 ff ff       	call   10c0 <_gfortran_st_read@plt>
    1fa8:	ba 04 00 00 00       	mov    $0x4,%edx
    1fad:	4c 89 ee             	mov    %r13,%rsi
    1fb0:	48 89 df             	mov    %rbx,%rdi
    1fb3:	e8 c8 f0 ff ff       	call   1080 <_gfortran_transfer_integer@plt>
    1fb8:	48 89 df             	mov    %rbx,%rdi
    1fbb:	e8 90 f0 ff ff       	call   1050 <_gfortran_st_read_done@plt>
    1fc0:	41 b8 0c 00 00 00    	mov    $0xc,%r8d
    1fc6:	31 c9                	xor    %ecx,%ecx
    1fc8:	31 c0                	xor    %eax,%eax
    1fca:	4c 89 e6             	mov    %r12,%rsi
    1fcd:	31 d2                	xor    %edx,%edx
    1fcf:	48 8d 3d de 11 00 00 	lea    0x11de(%rip),%rdi        # 31b4 <_IO_stdin_used+0x1b4>
    1fd6:	e8 45 f1 ff ff       	call   1120 <_gfortran_get_command_argument_i4@plt>
    1fdb:	48 89 df             	mov    %rbx,%rdi
    1fde:	4c 89 a5 30 fe ff ff 	mov    %r12,-0x1d0(%rbp)
    1fe5:	4c 89 bd c0 fd ff ff 	mov    %r15,-0x240(%rbp)
    1fec:	4c 89 b5 c8 fd ff ff 	mov    %r14,-0x238(%rbp)
    1ff3:	c7 85 d0 fd ff ff 60 	movl   $0x60,-0x230(%rbp)
    1ffa:	00 00 00 
    1ffd:	48 c7 85 38 fe ff ff 	movq   $0xc,-0x1c8(%rbp)
    2004:	0c 00 00 00 
    2008:	48 c7 85 08 fe ff ff 	movq   $0x0,-0x1f8(%rbp)
    200f:	00 00 00 00 
    2013:	e8 a8 f0 ff ff       	call   10c0 <_gfortran_st_read@plt>
    2018:	ba 04 00 00 00       	mov    $0x4,%edx
    201d:	48 8d b5 b8 fd ff ff 	lea    -0x248(%rbp),%rsi
    2024:	48 89 df             	mov    %rbx,%rdi
    2027:	e8 54 f0 ff ff       	call   1080 <_gfortran_transfer_integer@plt>
    202c:	48 89 df             	mov    %rbx,%rdi
    202f:	e8 1c f0 ff ff       	call   1050 <_gfortran_st_read_done@plt>
    2034:	be ff ff ff ff       	mov    $0xffffffff,%esi
    2039:	31 ff                	xor    %edi,%edi
    203b:	e8 b0 f0 ff ff       	call   10f0 <_gfortran_caf_num_images@plt>
    2040:	44 8b bd bc fd ff ff 	mov    -0x244(%rbp),%r15d
    2047:	41 89 c4             	mov    %eax,%r12d
    204a:	44 89 f8             	mov    %r15d,%eax
    204d:	99                   	cltd   
    204e:	41 f7 fc             	idiv   %r12d
    2051:	85 d2                	test   %edx,%edx
    2053:	0f 85 f7 06 00 00    	jne    2750 <MAIN__+0x900>
    2059:	48 b8 00 00 00 00 02 	movabs $0x30200000000,%rax
    2060:	03 00 00 
    2063:	48 c7 05 62 30 00 00 	movq   $0x8,0x3062(%rip)        # 50d0 <positions.1+0x10>
    206a:	08 00 00 00 
    206e:	48 89 05 63 30 00 00 	mov    %rax,0x3063(%rip)        # 50d8 <positions.1+0x18>
    2075:	44 89 f8             	mov    %r15d,%eax
    2078:	45 31 ff             	xor    %r15d,%r15d
    207b:	99                   	cltd   
    207c:	41 f7 fc             	idiv   %r12d
    207f:	ba 00 00 00 00       	mov    $0x0,%edx
    2084:	85 c0                	test   %eax,%eax
    2086:	44 0f 49 f8          	cmovns %eax,%r15d
    208a:	49 63 cf             	movslq %r15d,%rcx
    208d:	49 89 ce             	mov    %rcx,%r14
    2090:	49 f7 d6             	not    %r14
    2093:	7e 0c                	jle    20a1 <MAIN__+0x251>
    2095:	48 8d 0c 49          	lea    (%rcx,%rcx,2),%rcx
    2099:	48 8d 14 cd 00 00 00 	lea    0x0(,%rcx,8),%rdx
    20a0:	00 
    20a1:	48 83 3d 17 30 00 00 	cmpq   $0x0,0x3017(%rip)        # 50c0 <positions.1>
    20a8:	00 
    20a9:	0f 85 72 07 00 00    	jne    2821 <MAIN__+0x9d1>
    20af:	48 85 d2             	test   %rdx,%rdx
    20b2:	b8 01 00 00 00       	mov    $0x1,%eax
    20b7:	48 8d 0d 02 30 00 00 	lea    0x3002(%rip),%rcx        # 50c0 <positions.1>
    20be:	be 01 00 00 00       	mov    $0x1,%esi
    20c3:	48 0f 45 c2          	cmovne %rdx,%rax
    20c7:	48 83 ec 08          	sub    $0x8,%rsp
    20cb:	45 31 c9             	xor    %r9d,%r9d
    20ce:	45 31 c0             	xor    %r8d,%r8d
    20d1:	6a 00                	push   $0x0
    20d3:	48 89 c7             	mov    %rax,%rdi
    20d6:	48 8d 51 70          	lea    0x70(%rcx),%rdx
    20da:	e8 31 f0 ff ff       	call   1110 <_gfortran_caf_register@plt>
    20df:	8b 85 bc fd ff ff    	mov    -0x244(%rbp),%eax
    20e5:	c5 f9 6f 15 53 11 00 	vmovdqa 0x1153(%rip),%xmm2        # 3240 <options.103.4+0x80>
    20ec:	00 
    20ed:	31 f6                	xor    %esi,%esi
    20ef:	31 ff                	xor    %edi,%edi
    20f1:	c5 f9 6f 25 e7 10 00 	vmovdqa 0x10e7(%rip),%xmm4        # 31e0 <options.103.4+0x20>
    20f8:	00 
    20f9:	4c 89 35 c8 2f 00 00 	mov    %r14,0x2fc8(%rip)        # 50c8 <positions.1+0x8>
    2100:	48 c7 05 e5 2f 00 00 	movq   $0x1,0x2fe5(%rip)        # 50f0 <positions.1+0x30>
    2107:	01 00 00 00 
    210b:	48 c7 05 0a 30 00 00 	movq   $0x1,0x300a(%rip)        # 5120 <positions.1+0x60>
    2112:	01 00 00 00 
    2116:	99                   	cltd   
    2117:	41 f7 fc             	idiv   %r12d
    211a:	31 d2                	xor    %edx,%edx
    211c:	c5 fa 7f 15 e4 2f 00 	vmovdqu %xmm2,0x2fe4(%rip)        # 5108 <positions.1+0x48>
    2123:	00 
    2124:	c5 f9 7f 25 b4 2f 00 	vmovdqa %xmm4,0x2fb4(%rip)        # 50e0 <positions.1+0x20>
    212b:	00 
    212c:	c5 f9 6e d8          	vmovd  %eax,%xmm3
    2130:	89 85 a8 fd ff ff    	mov    %eax,-0x258(%rbp)
    2136:	c4 c3 61 22 c7 01    	vpinsrd $0x1,%r15d,%xmm3,%xmm0
    213c:	45 31 ff             	xor    %r15d,%r15d
    213f:	c4 e2 79 25 c0       	vpmovsxdq %xmm0,%xmm0
    2144:	c5 fa 7f 05 ac 2f 00 	vmovdqu %xmm0,0x2fac(%rip)        # 50f8 <positions.1+0x38>
    214b:	00 
    214c:	e8 5f ef ff ff       	call   10b0 <_gfortran_caf_sync_all@plt>
    2151:	48 b8 00 00 00 00 02 	movabs $0x30200000000,%rax
    2158:	03 00 00 
    215b:	5e                   	pop    %rsi
    215c:	48 89 05 f5 2e 00 00 	mov    %rax,0x2ef5(%rip)        # 5058 <velocities.0+0x18>
    2163:	8b 85 a8 fd ff ff    	mov    -0x258(%rbp),%eax
    2169:	48 c7 05 dc 2e 00 00 	movq   $0x8,0x2edc(%rip)        # 5050 <velocities.0+0x10>
    2170:	08 00 00 00 
    2174:	5f                   	pop    %rdi
    2175:	85 c0                	test   %eax,%eax
    2177:	44 0f 49 f8          	cmovns %eax,%r15d
    217b:	31 d2                	xor    %edx,%edx
    217d:	49 63 cf             	movslq %r15d,%rcx
    2180:	49 89 ce             	mov    %rcx,%r14
    2183:	49 f7 d6             	not    %r14
    2186:	85 c0                	test   %eax,%eax
    2188:	7e 0c                	jle    2196 <MAIN__+0x346>
    218a:	48 8d 0c 49          	lea    (%rcx,%rcx,2),%rcx
    218e:	48 8d 14 cd 00 00 00 	lea    0x0(,%rcx,8),%rdx
    2195:	00 
    2196:	48 83 3d a2 2e 00 00 	cmpq   $0x0,0x2ea2(%rip)        # 5040 <velocities.0>
    219d:	00 
    219e:	0f 85 61 06 00 00    	jne    2805 <MAIN__+0x9b5>
    21a4:	48 85 d2             	test   %rdx,%rdx
    21a7:	b8 01 00 00 00       	mov    $0x1,%eax
    21ac:	48 8d 0d 8d 2e 00 00 	lea    0x2e8d(%rip),%rcx        # 5040 <velocities.0>
    21b3:	be 01 00 00 00       	mov    $0x1,%esi
    21b8:	48 0f 45 c2          	cmovne %rdx,%rax
    21bc:	48 83 ec 08          	sub    $0x8,%rsp
    21c0:	45 31 c9             	xor    %r9d,%r9d
    21c3:	45 31 c0             	xor    %r8d,%r8d
    21c6:	6a 00                	push   $0x0
    21c8:	48 89 c7             	mov    %rax,%rdi
    21cb:	48 8d 51 70          	lea    0x70(%rcx),%rdx
    21cf:	e8 3c ef ff ff       	call   1110 <_gfortran_caf_register@plt>
    21d4:	8b 85 bc fd ff ff    	mov    -0x244(%rbp),%eax
    21da:	c5 f9 6f 2d 5e 10 00 	vmovdqa 0x105e(%rip),%xmm5        # 3240 <options.103.4+0x80>
    21e1:	00 
    21e2:	31 f6                	xor    %esi,%esi
    21e4:	31 ff                	xor    %edi,%edi
    21e6:	c5 f9 6f 3d f2 0f 00 	vmovdqa 0xff2(%rip),%xmm7        # 31e0 <options.103.4+0x20>
    21ed:	00 
    21ee:	4c 89 35 53 2e 00 00 	mov    %r14,0x2e53(%rip)        # 5048 <velocities.0+0x8>
    21f5:	48 c7 05 70 2e 00 00 	movq   $0x1,0x2e70(%rip)        # 5070 <velocities.0+0x30>
    21fc:	01 00 00 00 
    2200:	48 c7 05 95 2e 00 00 	movq   $0x1,0x2e95(%rip)        # 50a0 <velocities.0+0x60>
    2207:	01 00 00 00 
    220b:	99                   	cltd   
    220c:	41 f7 fc             	idiv   %r12d
    220f:	31 d2                	xor    %edx,%edx
    2211:	c5 fa 7f 2d 6f 2e 00 	vmovdqu %xmm5,0x2e6f(%rip)        # 5088 <velocities.0+0x48>
    2218:	00 
    2219:	c5 f9 7f 3d 3f 2e 00 	vmovdqa %xmm7,0x2e3f(%rip)        # 5060 <velocities.0+0x20>
    2220:	00 
    2221:	c5 f9 6e f0          	vmovd  %eax,%xmm6
    2225:	89 85 a8 fd ff ff    	mov    %eax,-0x258(%rbp)
    222b:	c4 c3 49 22 c7 01    	vpinsrd $0x1,%r15d,%xmm6,%xmm0
    2231:	45 31 ff             	xor    %r15d,%r15d
    2234:	c4 e2 79 25 c0       	vpmovsxdq %xmm0,%xmm0
    2239:	c5 fa 7f 05 37 2e 00 	vmovdqu %xmm0,0x2e37(%rip)        # 5078 <velocities.0+0x38>
    2240:	00 
    2241:	e8 6a ee ff ff       	call   10b0 <_gfortran_caf_sync_all@plt>
    2246:	48 b8 00 00 00 00 02 	movabs $0x30200000000,%rax
    224d:	03 00 00 
    2250:	41 5b                	pop    %r11
    2252:	48 89 05 5f 2f 00 00 	mov    %rax,0x2f5f(%rip)        # 51b8 <accel.3+0x18>
    2259:	8b 85 a8 fd ff ff    	mov    -0x258(%rbp),%eax
    225f:	48 c7 05 46 2f 00 00 	movq   $0x8,0x2f46(%rip)        # 51b0 <accel.3+0x10>
    2266:	08 00 00 00 
    226a:	5a                   	pop    %rdx
    226b:	85 c0                	test   %eax,%eax
    226d:	44 0f 49 f8          	cmovns %eax,%r15d
    2271:	31 d2                	xor    %edx,%edx
    2273:	49 63 cf             	movslq %r15d,%rcx
    2276:	49 89 ce             	mov    %rcx,%r14
    2279:	49 f7 d6             	not    %r14
    227c:	85 c0                	test   %eax,%eax
    227e:	7e 0c                	jle    228c <MAIN__+0x43c>
    2280:	48 8d 0c 49          	lea    (%rcx,%rcx,2),%rcx
    2284:	48 8d 14 cd 00 00 00 	lea    0x0(,%rcx,8),%rdx
    228b:	00 
    228c:	48 83 3d 0c 2f 00 00 	cmpq   $0x0,0x2f0c(%rip)        # 51a0 <accel.3>
    2293:	00 
    2294:	0f 85 4f 05 00 00    	jne    27e9 <MAIN__+0x999>
    229a:	48 85 d2             	test   %rdx,%rdx
    229d:	b8 01 00 00 00       	mov    $0x1,%eax
    22a2:	48 8d 0d f7 2e 00 00 	lea    0x2ef7(%rip),%rcx        # 51a0 <accel.3>
    22a9:	be 01 00 00 00       	mov    $0x1,%esi
    22ae:	48 0f 45 c2          	cmovne %rdx,%rax
    22b2:	48 83 ec 08          	sub    $0x8,%rsp
    22b6:	45 31 c9             	xor    %r9d,%r9d
    22b9:	45 31 c0             	xor    %r8d,%r8d
    22bc:	6a 00                	push   $0x0
    22be:	48 89 c7             	mov    %rax,%rdi
    22c1:	48 8d 51 70          	lea    0x70(%rcx),%rdx
    22c5:	e8 46 ee ff ff       	call   1110 <_gfortran_caf_register@plt>
    22ca:	8b 85 bc fd ff ff    	mov    -0x244(%rbp),%eax
    22d0:	c5 f9 6f 15 68 0f 00 	vmovdqa 0xf68(%rip),%xmm2        # 3240 <options.103.4+0x80>
    22d7:	00 
    22d8:	31 f6                	xor    %esi,%esi
    22da:	31 ff                	xor    %edi,%edi
    22dc:	c5 f9 6f 25 fc 0e 00 	vmovdqa 0xefc(%rip),%xmm4        # 31e0 <options.103.4+0x20>
    22e3:	00 
    22e4:	48 c7 05 e1 2e 00 00 	movq   $0x1,0x2ee1(%rip)        # 51d0 <accel.3+0x30>
    22eb:	01 00 00 00 
    22ef:	48 c7 05 06 2f 00 00 	movq   $0x1,0x2f06(%rip)        # 5200 <accel.3+0x60>
    22f6:	01 00 00 00 
    22fa:	4c 89 35 a7 2e 00 00 	mov    %r14,0x2ea7(%rip)        # 51a8 <accel.3+0x8>
    2301:	99                   	cltd   
    2302:	41 f7 fc             	idiv   %r12d
    2305:	31 d2                	xor    %edx,%edx
    2307:	c5 fa 7f 15 d9 2e 00 	vmovdqu %xmm2,0x2ed9(%rip)        # 51e8 <accel.3+0x48>
    230e:	00 
    230f:	c5 f9 7f 25 a9 2e 00 	vmovdqa %xmm4,0x2ea9(%rip)        # 51c0 <accel.3+0x20>
    2316:	00 
    2317:	c5 f9 6e d8          	vmovd  %eax,%xmm3
    231b:	89 85 a8 fd ff ff    	mov    %eax,-0x258(%rbp)
    2321:	c4 c3 61 22 c7 01    	vpinsrd $0x1,%r15d,%xmm3,%xmm0
    2327:	c4 e2 79 25 c0       	vpmovsxdq %xmm0,%xmm0
    232c:	c5 fa 7f 05 a4 2e 00 	vmovdqu %xmm0,0x2ea4(%rip)        # 51d8 <accel.3+0x38>
    2333:	00 
    2334:	e8 77 ed ff ff       	call   10b0 <_gfortran_caf_sync_all@plt>
    2339:	48 63 95 a8 fd ff ff 	movslq -0x258(%rbp),%rdx
    2340:	48 b8 00 00 00 00 01 	movabs $0x30100000000,%rax
    2347:	03 00 00 
    234a:	48 89 05 07 2e 00 00 	mov    %rax,0x2e07(%rip)        # 5158 <masses.2+0x18>
    2351:	41 59                	pop    %r9
    2353:	48 c7 05 f2 2d 00 00 	movq   $0x8,0x2df2(%rip)        # 5150 <masses.2+0x10>
    235a:	08 00 00 00 
    235e:	41 5a                	pop    %r10
    2360:	48 89 d0             	mov    %rdx,%rax
    2363:	48 c1 e2 03          	shl    $0x3,%rdx
    2367:	85 c0                	test   %eax,%eax
    2369:	b8 00 00 00 00       	mov    $0x0,%eax
    236e:	48 0f 4f c2          	cmovg  %rdx,%rax
    2372:	48 83 3d c6 2d 00 00 	cmpq   $0x0,0x2dc6(%rip)        # 5140 <masses.2>
    2379:	00 
    237a:	0f 85 4d 04 00 00    	jne    27cd <MAIN__+0x97d>
    2380:	48 85 c0             	test   %rax,%rax
    2383:	bf 01 00 00 00       	mov    $0x1,%edi
    2388:	48 8d 0d b1 2d 00 00 	lea    0x2db1(%rip),%rcx        # 5140 <masses.2>
    238f:	be 01 00 00 00       	mov    $0x1,%esi
    2394:	48 0f 45 f8          	cmovne %rax,%rdi
    2398:	48 83 ec 08          	sub    $0x8,%rsp
    239c:	45 31 c9             	xor    %r9d,%r9d
    239f:	45 31 c0             	xor    %r8d,%r8d
    23a2:	6a 00                	push   $0x0
    23a4:	48 8d 51 58          	lea    0x58(%rcx),%rdx
    23a8:	e8 63 ed ff ff       	call   1110 <_gfortran_caf_register@plt>
    23ad:	8b 85 bc fd ff ff    	mov    -0x244(%rbp),%eax
    23b3:	c5 f9 6f 2d 25 0e 00 	vmovdqa 0xe25(%rip),%xmm5        # 31e0 <options.103.4+0x20>
    23ba:	00 
    23bb:	31 f6                	xor    %esi,%esi
    23bd:	31 ff                	xor    %edi,%edi
    23bf:	48 c7 05 a6 2d 00 00 	movq   $0x1,0x2da6(%rip)        # 5170 <masses.2+0x30>
    23c6:	01 00 00 00 
    23ca:	48 c7 05 b3 2d 00 00 	movq   $0x1,0x2db3(%rip)        # 5188 <masses.2+0x48>
    23d1:	01 00 00 00 
    23d5:	48 c7 05 68 2d 00 00 	movq   $0xffffffffffffffff,0x2d68(%rip)        # 5148 <masses.2+0x8>
    23dc:	ff ff ff ff 
    23e0:	99                   	cltd   
    23e1:	41 f7 fc             	idiv   %r12d
    23e4:	31 d2                	xor    %edx,%edx
    23e6:	c5 f9 7f 2d 72 2d 00 	vmovdqa %xmm5,0x2d72(%rip)        # 5160 <masses.2+0x20>
    23ed:	00 
    23ee:	48 98                	cltq   
    23f0:	48 89 05 81 2d 00 00 	mov    %rax,0x2d81(%rip)        # 5178 <masses.2+0x38>
    23f7:	e8 b4 ec ff ff       	call   10b0 <_gfortran_caf_sync_all@plt>
    23fc:	48 8b 0d 05 2d 00 00 	mov    0x2d05(%rip),%rcx        # 5108 <positions.1+0x48>
    2403:	48 8b 15 06 2d 00 00 	mov    0x2d06(%rip),%rdx        # 5110 <positions.1+0x50>
    240a:	5f                   	pop    %rdi
    240b:	4c 8b 0d ae 2c 00 00 	mov    0x2cae(%rip),%r9        # 50c0 <positions.1>
    2412:	48 8b 35 af 2c 00 00 	mov    0x2caf(%rip),%rsi        # 50c8 <positions.1+0x8>
    2419:	48 8b 05 d0 2c 00 00 	mov    0x2cd0(%rip),%rax        # 50f0 <positions.1+0x30>
    2420:	4c 8b 15 d1 2c 00 00 	mov    0x2cd1(%rip),%r10        # 50f8 <positions.1+0x38>
    2427:	41 58                	pop    %r8
    2429:	48 39 d1             	cmp    %rdx,%rcx
    242c:	0f 8f 29 01 00 00    	jg     255b <MAIN__+0x70b>
    2432:	4c 8b 35 c7 2c 00 00 	mov    0x2cc7(%rip),%r14        # 5100 <positions.1+0x40>
    2439:	4c 39 d0             	cmp    %r10,%rax
    243c:	0f 8f 19 01 00 00    	jg     255b <MAIN__+0x70b>
    2442:	4c 89 f7             	mov    %r14,%rdi
    2445:	c5 fb 10 0d b3 0d 00 	vmovsd 0xdb3(%rip),%xmm1        # 3200 <options.103.4+0x40>
    244c:	00 
    244d:	c5 fd 28 05 ab 0d 00 	vmovapd 0xdab(%rip),%ymm0        # 3200 <options.103.4+0x40>
    2454:	00 
    2455:	4d 89 d7             	mov    %r10,%r15
    2458:	48 0f af f9          	imul   %rcx,%rdi
    245c:	48 ff c2             	inc    %rdx
    245f:	49 29 c7             	sub    %rax,%r15
    2462:	4d 8d 1c c1          	lea    (%r9,%rax,8),%r11
    2466:	48 29 ca             	sub    %rcx,%rdx
    2469:	49 8d 4f 01          	lea    0x1(%r15),%rcx
    246d:	4c 89 9d 90 fd ff ff 	mov    %r11,-0x270(%rbp)
    2474:	4c 89 ad 80 fd ff ff 	mov    %r13,-0x280(%rbp)
    247b:	48 89 95 88 fd ff ff 	mov    %rdx,-0x278(%rbp)
    2482:	48 89 8d a8 fd ff ff 	mov    %rcx,-0x258(%rbp)
    2489:	48 01 f7             	add    %rsi,%rdi
    248c:	48 89 ce             	mov    %rcx,%rsi
    248f:	48 83 e1 fc          	and    $0xfffffffffffffffc,%rcx
    2493:	4c 8b ad 90 fd ff ff 	mov    -0x270(%rbp),%r13
    249a:	48 89 9d 90 fd ff ff 	mov    %rbx,-0x270(%rbp)
    24a1:	48 8b 9d 88 fd ff ff 	mov    -0x278(%rbp),%rbx
    24a8:	48 c1 ee 02          	shr    $0x2,%rsi
    24ac:	48 8d 14 08          	lea    (%rax,%rcx,1),%rdx
    24b0:	49 83 ff 02          	cmp    $0x2,%r15
    24b4:	48 89 8d a0 fd ff ff 	mov    %rcx,-0x260(%rbp)
    24bb:	48 0f 46 d0          	cmovbe %rax,%rdx
    24bf:	45 31 c0             	xor    %r8d,%r8d
    24c2:	48 89 95 98 fd ff ff 	mov    %rdx,-0x268(%rbp)
    24c9:	4c 8d 5a 01          	lea    0x1(%rdx),%r11
    24cd:	4c 8d 62 02          	lea    0x2(%rdx),%r12
    24d1:	66 66 2e 0f 1f 84 00 	data16 cs nopw 0x0(%rax,%rax,1)
    24d8:	00 00 00 00 
    24dc:	0f 1f 40 00          	nopl   0x0(%rax)
    24e0:	49 83 ff 02          	cmp    $0x2,%r15
    24e4:	76 2e                	jbe    2514 <MAIN__+0x6c4>
    24e6:	49 8d 4c fd 00       	lea    0x0(%r13,%rdi,8),%rcx
    24eb:	31 c0                	xor    %eax,%eax
    24ed:	0f 1f 00             	nopl   (%rax)
    24f0:	48 89 c2             	mov    %rax,%rdx
    24f3:	48 ff c0             	inc    %rax
    24f6:	48 c1 e2 05          	shl    $0x5,%rdx
    24fa:	c5 fd 11 04 11       	vmovupd %ymm0,(%rcx,%rdx,1)
    24ff:	48 39 c6             	cmp    %rax,%rsi
    2502:	75 ec                	jne    24f0 <MAIN__+0x6a0>
    2504:	48 8b 8d a0 fd ff ff 	mov    -0x260(%rbp),%rcx
    250b:	48 39 8d a8 fd ff ff 	cmp    %rcx,-0x258(%rbp)
    2512:	74 2e                	je     2542 <MAIN__+0x6f2>
    2514:	48 8b 85 98 fd ff ff 	mov    -0x268(%rbp),%rax
    251b:	48 01 f8             	add    %rdi,%rax
    251e:	c4 c1 7b 11 0c c1    	vmovsd %xmm1,(%r9,%rax,8)
    2524:	4d 39 da             	cmp    %r11,%r10
    2527:	7c 19                	jl     2542 <MAIN__+0x6f2>
    2529:	4a 8d 04 1f          	lea    (%rdi,%r11,1),%rax
    252d:	c4 c1 7b 11 0c c1    	vmovsd %xmm1,(%r9,%rax,8)
    2533:	4d 39 e2             	cmp    %r12,%r10
    2536:	7c 0a                	jl     2542 <MAIN__+0x6f2>
    2538:	4a 8d 04 27          	lea    (%rdi,%r12,1),%rax
    253c:	c4 c1 7b 11 0c c1    	vmovsd %xmm1,(%r9,%rax,8)
    2542:	49 ff c0             	inc    %r8
    2545:	4c 01 f7             	add    %r14,%rdi
    2548:	4c 39 c3             	cmp    %r8,%rbx
    254b:	75 93                	jne    24e0 <MAIN__+0x690>
    254d:	4c 8b ad 80 fd ff ff 	mov    -0x280(%rbp),%r13
    2554:	48 8b 9d 90 fd ff ff 	mov    -0x270(%rbp),%rbx
    255b:	48 8b 15 0e 2c 00 00 	mov    0x2c0e(%rip),%rdx        # 5170 <masses.2+0x30>
    2562:	48 8b 3d 0f 2c 00 00 	mov    0x2c0f(%rip),%rdi        # 5178 <masses.2+0x38>
    2569:	4c 8b 05 d0 2b 00 00 	mov    0x2bd0(%rip),%r8        # 5140 <masses.2>
    2570:	4c 8b 0d d1 2b 00 00 	mov    0x2bd1(%rip),%r9        # 5148 <masses.2+0x8>
    2577:	48 39 fa             	cmp    %rdi,%rdx
    257a:	0f 8f 89 00 00 00    	jg     2609 <MAIN__+0x7b9>
    2580:	48 89 f8             	mov    %rdi,%rax
    2583:	48 29 d0             	sub    %rdx,%rax
    2586:	4c 8d 58 01          	lea    0x1(%rax),%r11
    258a:	48 83 f8 02          	cmp    $0x2,%rax
    258e:	76 43                	jbe    25d3 <MAIN__+0x783>
    2590:	c5 fd 28 05 68 0c 00 	vmovapd 0xc68(%rip),%ymm0        # 3200 <options.103.4+0x40>
    2597:	00 
    2598:	49 8d 04 11          	lea    (%r9,%rdx,1),%rax
    259c:	4d 89 da             	mov    %r11,%r10
    259f:	49 8d 34 c0          	lea    (%r8,%rax,8),%rsi
    25a3:	49 c1 ea 02          	shr    $0x2,%r10
    25a7:	31 c0                	xor    %eax,%eax
    25a9:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    25b0:	48 89 c1             	mov    %rax,%rcx
    25b3:	48 ff c0             	inc    %rax
    25b6:	48 c1 e1 05          	shl    $0x5,%rcx
    25ba:	c5 fd 11 04 0e       	vmovupd %ymm0,(%rsi,%rcx,1)
    25bf:	49 39 c2             	cmp    %rax,%r10
    25c2:	75 ec                	jne    25b0 <MAIN__+0x760>
    25c4:	4c 89 d8             	mov    %r11,%rax
    25c7:	48 83 e0 fc          	and    $0xfffffffffffffffc,%rax
    25cb:	48 01 c2             	add    %rax,%rdx
    25ce:	4c 39 d8             	cmp    %r11,%rax
    25d1:	74 36                	je     2609 <MAIN__+0x7b9>
    25d3:	c5 fb 10 05 25 0c 00 	vmovsd 0xc25(%rip),%xmm0        # 3200 <options.103.4+0x40>
    25da:	00 
    25db:	4a 8d 04 0a          	lea    (%rdx,%r9,1),%rax
    25df:	c4 c1 7b 11 04 c0    	vmovsd %xmm0,(%r8,%rax,8)
    25e5:	48 8d 42 01          	lea    0x1(%rdx),%rax
    25e9:	48 39 f8             	cmp    %rdi,%rax
    25ec:	7f 1b                	jg     2609 <MAIN__+0x7b9>
    25ee:	4c 01 c8             	add    %r9,%rax
    25f1:	48 83 c2 02          	add    $0x2,%rdx
    25f5:	c4 c1 7b 11 04 c0    	vmovsd %xmm0,(%r8,%rax,8)
    25fb:	48 39 fa             	cmp    %rdi,%rdx
    25fe:	7f 09                	jg     2609 <MAIN__+0x7b9>
    2600:	4c 01 ca             	add    %r9,%rdx
    2603:	c4 c1 7b 11 04 d0    	vmovsd %xmm0,(%r8,%rdx,8)
    2609:	48 8b 35 78 2a 00 00 	mov    0x2a78(%rip),%rsi        # 5088 <velocities.0+0x48>
    2610:	4c 8b 25 79 2a 00 00 	mov    0x2a79(%rip),%r12        # 5090 <velocities.0+0x50>
    2617:	48 8b 3d 22 2a 00 00 	mov    0x2a22(%rip),%rdi        # 5040 <velocities.0>
    261e:	4c 8b 05 23 2a 00 00 	mov    0x2a23(%rip),%r8        # 5048 <velocities.0+0x8>
    2625:	48 8b 0d 44 2a 00 00 	mov    0x2a44(%rip),%rcx        # 5070 <velocities.0+0x30>
    262c:	48 8b 15 45 2a 00 00 	mov    0x2a45(%rip),%rdx        # 5078 <velocities.0+0x38>
    2633:	4c 39 e6             	cmp    %r12,%rsi
    2636:	0f 8f 6f 01 00 00    	jg     27ab <MAIN__+0x95b>
    263c:	48 8b 05 3d 2a 00 00 	mov    0x2a3d(%rip),%rax        # 5080 <velocities.0+0x40>
    2643:	48 39 d1             	cmp    %rdx,%rcx
    2646:	0f 8f 5f 01 00 00    	jg     27ab <MAIN__+0x95b>
    264c:	4c 8d 3c c5 00 00 00 	lea    0x0(,%rax,8),%r15
    2653:	00 
    2654:	48 0f af c6          	imul   %rsi,%rax
    2658:	49 ff c4             	inc    %r12
    265b:	48 89 9d a8 fd ff ff 	mov    %rbx,-0x258(%rbp)
    2662:	49 29 f4             	sub    %rsi,%r12
    2665:	4c 89 e6             	mov    %r12,%rsi
    2668:	45 31 e4             	xor    %r12d,%r12d
    266b:	4c 01 c0             	add    %r8,%rax
    266e:	4c 89 e3             	mov    %r12,%rbx
    2671:	49 89 f4             	mov    %rsi,%r12
    2674:	48 01 c8             	add    %rcx,%rax
    2677:	48 8d 3c c7          	lea    (%rdi,%rax,8),%rdi
    267b:	48 89 d0             	mov    %rdx,%rax
    267e:	48 29 c8             	sub    %rcx,%rax
    2681:	4c 8d 34 c5 08 00 00 	lea    0x8(,%rax,8),%r14
    2688:	00 
    2689:	c5 f8 77             	vzeroupper 
    268c:	0f 1f 40 00          	nopl   0x0(%rax)
    2690:	4c 89 f2             	mov    %r14,%rdx
    2693:	31 f6                	xor    %esi,%esi
    2695:	48 ff c3             	inc    %rbx
    2698:	e8 93 e9 ff ff       	call   1030 <memset@plt>
    269d:	48 89 c7             	mov    %rax,%rdi
    26a0:	4c 01 ff             	add    %r15,%rdi
    26a3:	49 39 dc             	cmp    %rbx,%r12
    26a6:	75 e8                	jne    2690 <MAIN__+0x840>
    26a8:	48 8b 9d a8 fd ff ff 	mov    -0x258(%rbp),%rbx
    26af:	48 8b 05 7a 0b 00 00 	mov    0xb7a(%rip),%rax        # 3230 <options.103.4+0x70>
    26b6:	48 89 85 c0 fd ff ff 	mov    %rax,-0x240(%rbp)
    26bd:	31 d2                	xor    %edx,%edx
    26bf:	31 f6                	xor    %esi,%esi
    26c1:	31 ff                	xor    %edi,%edi
    26c3:	e8 e8 e9 ff ff       	call   10b0 <_gfortran_caf_sync_all@plt>
    26c8:	44 8b a5 b8 fd ff ff 	mov    -0x248(%rbp),%r12d
    26cf:	45 85 e4             	test   %r12d,%r12d
    26d2:	7e 6b                	jle    273f <MAIN__+0x8ef>
    26d4:	41 be 01 00 00 00    	mov    $0x1,%r14d
    26da:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
    26e0:	48 8d 05 b9 2a 00 00 	lea    0x2ab9(%rip),%rax        # 51a0 <accel.3>
    26e7:	6a 00                	push   $0x0
    26e9:	ff 35 21 2b 00 00    	push   0x2b21(%rip)        # 5210 <accel.3+0x70>
    26ef:	6a 00                	push   $0x0
    26f1:	ff 35 a1 2a 00 00    	push   0x2aa1(%rip)        # 5198 <masses.2+0x58>
    26f7:	6a 00                	push   $0x0
    26f9:	4d 89 e9             	mov    %r13,%r9
    26fc:	49 89 d8             	mov    %rbx,%r8
    26ff:	ff 35 ab 29 00 00    	push   0x29ab(%rip)        # 50b0 <velocities.0+0x70>
    2705:	6a 00                	push   $0x0
    2707:	41 ff c6             	inc    %r14d
    270a:	ff 35 20 2a 00 00    	push   0x2a20(%rip)        # 5130 <positions.1+0x70>
    2710:	48 8b 08             	mov    (%rax),%rcx
    2713:	48 8d 05 26 2a 00 00 	lea    0x2a26(%rip),%rax        # 5140 <masses.2>
    271a:	48 8b 10             	mov    (%rax),%rdx
    271d:	48 8d 05 1c 29 00 00 	lea    0x291c(%rip),%rax        # 5040 <velocities.0>
    2724:	48 8b 30             	mov    (%rax),%rsi
    2727:	48 8d 05 92 29 00 00 	lea    0x2992(%rip),%rax        # 50c0 <positions.1>
    272e:	48 8b 38             	mov    (%rax),%rdi
    2731:	e8 da f3 ff ff       	call   1b10 <__nbodymodule_MOD_advanceit>
    2736:	48 83 c4 40          	add    $0x40,%rsp
    273a:	45 39 f4             	cmp    %r14d,%r12d
    273d:	7d a1                	jge    26e0 <MAIN__+0x890>
    273f:	48 8d 65 d0          	lea    -0x30(%rbp),%rsp
    2743:	5b                   	pop    %rbx
    2744:	41 5a                	pop    %r10
    2746:	41 5c                	pop    %r12
    2748:	41 5d                	pop    %r13
    274a:	41 5e                	pop    %r14
    274c:	41 5f                	pop    %r15
    274e:	5d                   	pop    %rbp
    274f:	c3                   	ret    
    2750:	44 89 f8             	mov    %r15d,%eax
    2753:	44 31 e0             	xor    %r12d,%eax
    2756:	79 03                	jns    275b <MAIN__+0x90b>
    2758:	44 01 e2             	add    %r12d,%edx
    275b:	85 d2                	test   %edx,%edx
    275d:	0f 84 f6 f8 ff ff    	je     2059 <MAIN__+0x209>
    2763:	48 8b 05 be 0a 00 00 	mov    0xabe(%rip),%rax        # 3228 <options.103.4+0x68>
    276a:	48 89 df             	mov    %rbx,%rdi
    276d:	4c 89 b5 c8 fd ff ff 	mov    %r14,-0x238(%rbp)
    2774:	c7 85 d0 fd ff ff 65 	movl   $0x65,-0x230(%rbp)
    277b:	00 00 00 
    277e:	48 89 85 c0 fd ff ff 	mov    %rax,-0x240(%rbp)
    2785:	e8 d6 e9 ff ff       	call   1160 <_gfortran_st_write@plt>
    278a:	48 89 df             	mov    %rbx,%rdi
    278d:	ba 1c 00 00 00       	mov    $0x1c,%edx
    2792:	48 8d 35 a1 08 00 00 	lea    0x8a1(%rip),%rsi        # 303a <_IO_stdin_used+0x3a>
    2799:	e8 f2 e8 ff ff       	call   1090 <_gfortran_transfer_character_write@plt>
    279e:	48 89 df             	mov    %rbx,%rdi
    27a1:	e8 3a e9 ff ff       	call   10e0 <_gfortran_st_write_done@plt>
    27a6:	e9 ae f8 ff ff       	jmp    2059 <MAIN__+0x209>
    27ab:	c5 f8 77             	vzeroupper 
    27ae:	e9 fc fe ff ff       	jmp    26af <MAIN__+0x85f>
    27b3:	ba 18 00 00 00       	mov    $0x18,%edx
    27b8:	48 8d 35 45 08 00 00 	lea    0x845(%rip),%rsi        # 3004 <_IO_stdin_used+0x4>
    27bf:	48 8d 3d ba 08 00 00 	lea    0x8ba(%rip),%rdi        # 3080 <_IO_stdin_used+0x80>
    27c6:	31 c0                	xor    %eax,%eax
    27c8:	e8 d3 e8 ff ff       	call   10a0 <_gfortran_os_error_at@plt>
    27cd:	48 8d 15 9e 08 00 00 	lea    0x89e(%rip),%rdx        # 3072 <_IO_stdin_used+0x72>
    27d4:	48 8d 35 dd 08 00 00 	lea    0x8dd(%rip),%rsi        # 30b8 <_IO_stdin_used+0xb8>
    27db:	48 8d 3d 9e 09 00 00 	lea    0x99e(%rip),%rdi        # 3180 <_IO_stdin_used+0x180>
    27e2:	31 c0                	xor    %eax,%eax
    27e4:	e8 57 e8 ff ff       	call   1040 <_gfortran_runtime_error_at@plt>
    27e9:	48 8d 15 7c 08 00 00 	lea    0x87c(%rip),%rdx        # 306c <_IO_stdin_used+0x6c>
    27f0:	48 8d 35 c1 08 00 00 	lea    0x8c1(%rip),%rsi        # 30b8 <_IO_stdin_used+0xb8>
    27f7:	48 8d 3d 52 09 00 00 	lea    0x952(%rip),%rdi        # 3150 <_IO_stdin_used+0x150>
    27fe:	31 c0                	xor    %eax,%eax
    2800:	e8 3b e8 ff ff       	call   1040 <_gfortran_runtime_error_at@plt>
    2805:	48 8d 15 55 08 00 00 	lea    0x855(%rip),%rdx        # 3061 <_IO_stdin_used+0x61>
    280c:	48 8d 35 a5 08 00 00 	lea    0x8a5(%rip),%rsi        # 30b8 <_IO_stdin_used+0xb8>
    2813:	48 8d 3d 06 09 00 00 	lea    0x906(%rip),%rdi        # 3120 <_IO_stdin_used+0x120>
    281a:	31 c0                	xor    %eax,%eax
    281c:	e8 1f e8 ff ff       	call   1040 <_gfortran_runtime_error_at@plt>
    2821:	48 8d 15 2f 08 00 00 	lea    0x82f(%rip),%rdx        # 3057 <_IO_stdin_used+0x57>
    2828:	48 8d 35 89 08 00 00 	lea    0x889(%rip),%rsi        # 30b8 <_IO_stdin_used+0xb8>
    282f:	48 8d 3d ba 08 00 00 	lea    0x8ba(%rip),%rdi        # 30f0 <_IO_stdin_used+0xf0>
    2836:	31 c0                	xor    %eax,%eax
    2838:	e8 03 e8 ff ff       	call   1040 <_gfortran_runtime_error_at@plt>

Disassembly of section .fini:

0000000000002840 <_fini>:
    2840:	f3 0f 1e fa          	endbr64 
    2844:	48 83 ec 08          	sub    $0x8,%rsp
    2848:	48 83 c4 08          	add    $0x8,%rsp
    284c:	c3                   	ret    
