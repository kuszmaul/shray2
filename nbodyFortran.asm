
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
    11b6:	48 8d 35 d3 1f 00 00 	lea    0x1fd3(%rip),%rsi        # 3190 <options.97.4>
    11bd:	bf 07 00 00 00       	mov    $0x7,%edi
    11c2:	e8 39 ff ff ff       	call   1100 <_gfortran_set_options@plt>
    11c7:	e8 14 0b 00 00       	call   1ce0 <MAIN__>
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
    130c:	49 89 fd             	mov    %rdi,%r13
    130f:	48 81 ec 40 02 00 00 	sub    $0x240,%rsp
    1316:	49 8b 1a             	mov    (%r10),%rbx
    1319:	49 8b 42 18          	mov    0x18(%r10),%rax
    131d:	31 ff                	xor    %edi,%edi
    131f:	48 89 b5 48 fe ff ff 	mov    %rsi,-0x1b8(%rbp)
    1326:	be ff ff ff ff       	mov    $0xffffffff,%esi
    132b:	48 89 95 40 fe ff ff 	mov    %rdx,-0x1c0(%rbp)
    1332:	48 89 8d c8 fd ff ff 	mov    %rcx,-0x238(%rbp)
    1339:	48 89 9d f8 fd ff ff 	mov    %rbx,-0x208(%rbp)
    1340:	49 8b 5a 08          	mov    0x8(%r10),%rbx
    1344:	48 89 85 e0 fd ff ff 	mov    %rax,-0x220(%rbp)
    134b:	48 89 9d f0 fd ff ff 	mov    %rbx,-0x210(%rbp)
    1352:	49 8b 5a 10          	mov    0x10(%r10),%rbx
    1356:	48 89 9d e8 fd ff ff 	mov    %rbx,-0x218(%rbp)
    135d:	8b 19                	mov    (%rcx),%ebx
    135f:	e8 8c fd ff ff       	call   10f0 <_gfortran_caf_num_images@plt>
    1364:	be ff ff ff ff       	mov    $0xffffffff,%esi
    1369:	41 89 c0             	mov    %eax,%r8d
    136c:	89 d8                	mov    %ebx,%eax
    136e:	99                   	cltd   
    136f:	41 f7 f8             	idiv   %r8d
    1372:	48 63 c8             	movslq %eax,%rcx
    1375:	31 c0                	xor    %eax,%eax
    1377:	48 85 c9             	test   %rcx,%rcx
    137a:	48 89 8d 78 fe ff ff 	mov    %rcx,-0x188(%rbp)
    1381:	48 0f 49 c1          	cmovns %rcx,%rax
    1385:	31 ff                	xor    %edi,%edi
    1387:	49 89 c6             	mov    %rax,%r14
    138a:	49 89 c4             	mov    %rax,%r12
    138d:	e8 5e fd ff ff       	call   10f0 <_gfortran_caf_num_images@plt>
    1392:	be ff ff ff ff       	mov    $0xffffffff,%esi
    1397:	41 89 c0             	mov    %eax,%r8d
    139a:	89 d8                	mov    %ebx,%eax
    139c:	49 f7 d6             	not    %r14
    139f:	99                   	cltd   
    13a0:	41 f7 f8             	idiv   %r8d
    13a3:	48 63 d0             	movslq %eax,%rdx
    13a6:	89 85 58 fe ff ff    	mov    %eax,-0x1a8(%rbp)
    13ac:	31 c0                	xor    %eax,%eax
    13ae:	48 85 d2             	test   %rdx,%rdx
    13b1:	48 0f 49 c2          	cmovns %rdx,%rax
    13b5:	31 ff                	xor    %edi,%edi
    13b7:	48 89 85 50 fe ff ff 	mov    %rax,-0x1b0(%rbp)
    13be:	48 f7 d0             	not    %rax
    13c1:	49 89 c7             	mov    %rax,%r15
    13c4:	e8 27 fd ff ff       	call   10f0 <_gfortran_caf_num_images@plt>
    13c9:	48 8b 8d 78 fe ff ff 	mov    -0x188(%rbp),%rcx
    13d0:	48 85 c9             	test   %rcx,%rcx
    13d3:	7e 67                	jle    143c <__acceleratemodule_MOD_accelerateall+0x14c>
    13d5:	4e 8d 0c e5 00 00 00 	lea    0x0(,%r12,8),%r9
    13dc:	00 
    13dd:	48 c1 e1 03          	shl    $0x3,%rcx
    13e1:	41 b8 03 00 00 00    	mov    $0x3,%r8d
    13e7:	4c 89 ef             	mov    %r13,%rdi
    13ea:	4c 89 bd 78 fe ff ff 	mov    %r15,-0x188(%rbp)
    13f1:	89 9d 70 fe ff ff    	mov    %ebx,-0x190(%rbp)
    13f7:	4d 89 f7             	mov    %r14,%r15
    13fa:	4c 89 ad 68 fe ff ff 	mov    %r13,-0x198(%rbp)
    1401:	4d 89 e6             	mov    %r12,%r14
    1404:	4c 89 c3             	mov    %r8,%rbx
    1407:	49 89 cc             	mov    %rcx,%r12
    140a:	4d 89 cd             	mov    %r9,%r13
    140d:	31 f6                	xor    %esi,%esi
    140f:	4c 89 e2             	mov    %r12,%rdx
    1412:	e8 19 fc ff ff       	call   1030 <memset@plt>
    1417:	48 89 c7             	mov    %rax,%rdi
    141a:	4c 01 ef             	add    %r13,%rdi
    141d:	48 ff cb             	dec    %rbx
    1420:	75 eb                	jne    140d <__acceleratemodule_MOD_accelerateall+0x11d>
    1422:	4d 89 f4             	mov    %r14,%r12
    1425:	8b 9d 70 fe ff ff    	mov    -0x190(%rbp),%ebx
    142b:	4d 89 fe             	mov    %r15,%r14
    142e:	4c 8b ad 68 fe ff ff 	mov    -0x198(%rbp),%r13
    1435:	4c 8b bd 78 fe ff ff 	mov    -0x188(%rbp),%r15
    143c:	be ff ff ff ff       	mov    $0xffffffff,%esi
    1441:	31 ff                	xor    %edi,%edi
    1443:	e8 a8 fc ff ff       	call   10f0 <_gfortran_caf_num_images@plt>
    1448:	89 85 d0 fd ff ff    	mov    %eax,-0x230(%rbp)
    144e:	85 c0                	test   %eax,%eax
    1450:	0f 8e 87 04 00 00    	jle    18dd <__acceleratemodule_MOD_accelerateall+0x5ed>
    1456:	49 8d 45 f8          	lea    -0x8(%r13),%rax
    145a:	48 8b bd 48 fe ff ff 	mov    -0x1b8(%rbp),%rdi
    1461:	48 89 85 98 fd ff ff 	mov    %rax,-0x268(%rbp)
    1468:	4b 8d 04 24          	lea    (%r12,%r12,1),%rax
    146c:	48 8b 8d 50 fe ff ff 	mov    -0x1b0(%rbp),%rcx
    1473:	4a 8d 14 30          	lea    (%rax,%r14,1),%rdx
    1477:	4c 01 e0             	add    %r12,%rax
    147a:	4c 01 f0             	add    %r14,%rax
    147d:	49 8d 54 d5 00       	lea    0x0(%r13,%rdx,8),%rdx
    1482:	41 be 01 00 00 00    	mov    $0x1,%r14d
    1488:	49 8d 44 c5 00       	lea    0x0(%r13,%rax,8),%rax
    148d:	48 89 95 a8 fd ff ff 	mov    %rdx,-0x258(%rbp)
    1494:	48 89 85 a0 fd ff ff 	mov    %rax,-0x260(%rbp)
    149b:	48 8d 47 f8          	lea    -0x8(%rdi),%rax
    149f:	48 89 85 c0 fd ff ff 	mov    %rax,-0x240(%rbp)
    14a6:	48 8d 04 09          	lea    (%rcx,%rcx,1),%rax
    14aa:	4a 8d 14 38          	lea    (%rax,%r15,1),%rdx
    14ae:	48 01 c8             	add    %rcx,%rax
    14b1:	4c 01 f8             	add    %r15,%rax
    14b4:	48 8d 14 d7          	lea    (%rdi,%rdx,8),%rdx
    14b8:	48 8d 04 c7          	lea    (%rdi,%rax,8),%rax
    14bc:	48 89 95 b8 fd ff ff 	mov    %rdx,-0x248(%rbp)
    14c3:	48 89 85 b0 fd ff ff 	mov    %rax,-0x250(%rbp)
    14ca:	be ff ff ff ff       	mov    $0xffffffff,%esi
    14cf:	31 ff                	xor    %edi,%edi
    14d1:	e8 1a fc ff ff       	call   10f0 <_gfortran_caf_num_images@plt>
    14d6:	41 89 c0             	mov    %eax,%r8d
    14d9:	89 d8                	mov    %ebx,%eax
    14db:	99                   	cltd   
    14dc:	41 f7 f8             	idiv   %r8d
    14df:	89 85 d4 fd ff ff    	mov    %eax,-0x22c(%rbp)
    14e5:	85 c0                	test   %eax,%eax
    14e7:	0f 8e d6 03 00 00    	jle    18c3 <__acceleratemodule_MOD_accelerateall+0x5d3>
    14ed:	48 8b 85 a0 fd ff ff 	mov    -0x260(%rbp),%rax
    14f4:	48 c7 85 d8 fd ff ff 	movq   $0x0,-0x228(%rbp)
    14fb:	00 00 00 00 
    14ff:	48 89 85 70 fe ff ff 	mov    %rax,-0x190(%rbp)
    1506:	48 8b 85 a8 fd ff ff 	mov    -0x258(%rbp),%rax
    150d:	48 89 85 68 fe ff ff 	mov    %rax,-0x198(%rbp)
    1514:	48 8b 85 98 fd ff ff 	mov    -0x268(%rbp),%rax
    151b:	48 89 85 60 fe ff ff 	mov    %rax,-0x1a0(%rbp)
    1522:	66 66 2e 0f 1f 84 00 	data16 cs nopw 0x0(%rax,%rax,1)
    1529:	00 00 00 00 
    152d:	0f 1f 00             	nopl   (%rax)
    1530:	be ff ff ff ff       	mov    $0xffffffff,%esi
    1535:	31 ff                	xor    %edi,%edi
    1537:	e8 b4 fb ff ff       	call   10f0 <_gfortran_caf_num_images@plt>
    153c:	41 89 c0             	mov    %eax,%r8d
    153f:	89 d8                	mov    %ebx,%eax
    1541:	99                   	cltd   
    1542:	41 f7 f8             	idiv   %r8d
    1545:	89 85 5c fe ff ff    	mov    %eax,-0x1a4(%rbp)
    154b:	85 c0                	test   %eax,%eax
    154d:	0f 8e e8 02 00 00    	jle    183b <__acceleratemodule_MOD_accelerateall+0x54b>
    1553:	83 bd 58 fe ff ff 01 	cmpl   $0x1,-0x1a8(%rbp)
    155a:	48 8d 9d 90 fe ff ff 	lea    -0x170(%rbp),%rbx
    1561:	4c 8d ad 30 ff ff ff 	lea    -0xd0(%rbp),%r13
    1568:	48 8b 85 48 fe ff ff 	mov    -0x1b8(%rbp),%rax
    156f:	48 8b bd d8 fd ff ff 	mov    -0x228(%rbp),%rdi
    1576:	4c 8b a5 40 fe ff ff 	mov    -0x1c0(%rbp),%r12
    157d:	48 8d 04 f8          	lea    (%rax,%rdi,8),%rax
    1581:	48 0f 44 d8          	cmove  %rax,%rbx
    1585:	48 8b 85 50 fe ff ff 	mov    -0x1b0(%rbp),%rax
    158c:	45 31 ff             	xor    %r15d,%r15d
    158f:	48 f7 d8             	neg    %rax
    1592:	48 89 85 38 fe ff ff 	mov    %rax,-0x1c8(%rbp)
    1599:	48 8d 85 b0 fe ff ff 	lea    -0x150(%rbp),%rax
    15a0:	48 89 85 28 fe ff ff 	mov    %rax,-0x1d8(%rbp)
    15a7:	48 8d 45 80          	lea    -0x80(%rbp),%rax
    15ab:	48 89 85 30 fe ff ff 	mov    %rax,-0x1d0(%rbp)
    15b2:	48 8d 85 88 fe ff ff 	lea    -0x178(%rbp),%rax
    15b9:	48 89 85 10 fe ff ff 	mov    %rax,-0x1f0(%rbp)
    15c0:	48 8d 85 d0 fe ff ff 	lea    -0x130(%rbp),%rax
    15c7:	48 89 85 18 fe ff ff 	mov    %rax,-0x1e8(%rbp)
    15ce:	48 8d 85 00 ff ff ff 	lea    -0x100(%rbp),%rax
    15d5:	48 89 85 20 fe ff ff 	mov    %rax,-0x1e0(%rbp)
    15dc:	4c 89 e8             	mov    %r13,%rax
    15df:	4d 89 fd             	mov    %r15,%r13
    15e2:	49 89 c7             	mov    %rax,%r15
    15e5:	66 66 2e 0f 1f 84 00 	data16 cs nopw 0x0(%rax,%rax,1)
    15ec:	00 00 00 00 
    15f0:	83 bd 58 fe ff ff 01 	cmpl   $0x1,-0x1a8(%rbp)
    15f7:	0f 85 83 02 00 00    	jne    1880 <__acceleratemodule_MOD_accelerateall+0x590>
    15fd:	48 8b 85 50 fe ff ff 	mov    -0x1b0(%rbp),%rax
    1604:	48 8b 8d 48 fe ff ff 	mov    -0x1b8(%rbp),%rcx
    160b:	48 c7 45 b0 01 00 00 	movq   $0x1,-0x50(%rbp)
    1612:	00 
    1613:	48 c7 45 b8 03 00 00 	movq   $0x3,-0x48(%rbp)
    161a:	00 
    161b:	48 c7 45 90 08 00 00 	movq   $0x8,-0x70(%rbp)
    1622:	00 
    1623:	48 c7 45 a0 08 00 00 	movq   $0x8,-0x60(%rbp)
    162a:	00 
    162b:	48 c7 85 38 ff ff ff 	movq   $0x0,-0xc8(%rbp)
    1632:	00 00 00 00 
    1636:	48 c7 85 40 ff ff ff 	movq   $0x8,-0xc0(%rbp)
    163d:	08 00 00 00 
    1641:	c5 fd 6f 2d 77 1b 00 	vmovdqa 0x1b77(%rip),%ymm5        # 31c0 <options.97.4+0x30>
    1648:	00 
    1649:	48 89 45 a8          	mov    %rax,-0x58(%rbp)
    164d:	4a 8d 04 e9          	lea    (%rcx,%r13,8),%rax
    1651:	48 8b 95 28 fe ff ff 	mov    -0x1d8(%rbp),%rdx
    1658:	48 89 45 80          	mov    %rax,-0x80(%rbp)
    165c:	48 8b 85 38 fe ff ff 	mov    -0x1c8(%rbp),%rax
    1663:	48 89 95 30 ff ff ff 	mov    %rdx,-0xd0(%rbp)
    166a:	48 89 45 88          	mov    %rax,-0x78(%rbp)
    166e:	48 b8 00 00 00 00 01 	movabs $0x30100000000,%rax
    1675:	03 00 00 
    1678:	48 89 45 98          	mov    %rax,-0x68(%rbp)
    167c:	48 89 85 48 ff ff ff 	mov    %rax,-0xb8(%rbp)
    1683:	c5 fd 7f ad 50 ff ff 	vmovdqa %ymm5,-0xb0(%rbp)
    168a:	ff 
    168b:	48 8b b5 f0 fd ff ff 	mov    -0x210(%rbp),%rsi
    1692:	48 03 75 80          	add    -0x80(%rbp),%rsi
    1696:	6a 00                	push   $0x0
    1698:	4d 89 f9             	mov    %r15,%r9
    169b:	48 8b bd f8 fd ff ff 	mov    -0x208(%rbp),%rdi
    16a2:	6a 00                	push   $0x0
    16a4:	45 31 c0             	xor    %r8d,%r8d
    16a7:	6a 08                	push   $0x8
    16a9:	44 89 f2             	mov    %r14d,%edx
    16ac:	6a 08                	push   $0x8
    16ae:	48 29 ce             	sub    %rcx,%rsi
    16b1:	48 8b 8d 30 fe ff ff 	mov    -0x1d0(%rbp),%rcx
    16b8:	c5 f8 77             	vzeroupper 
    16bb:	e8 90 fa ff ff       	call   1150 <_gfortran_caf_get@plt>
    16c0:	48 83 c4 20          	add    $0x20,%rsp
    16c4:	4c 89 ff             	mov    %r15,%rdi
    16c7:	e8 04 fa ff ff       	call   10d0 <_gfortran_internal_pack@plt>
    16cc:	48 8b bd 10 fe ff ff 	mov    -0x1f0(%rbp),%rdi
    16d3:	48 c7 85 e0 fe ff ff 	movq   $0x8,-0x120(%rbp)
    16da:	08 00 00 00 
    16de:	48 c7 85 10 ff ff ff 	movq   $0x8,-0xf0(%rbp)
    16e5:	08 00 00 00 
    16e9:	48 89 85 78 fe ff ff 	mov    %rax,-0x188(%rbp)
    16f0:	48 b8 00 00 00 00 00 	movabs $0x30000000000,%rax
    16f7:	03 00 00 
    16fa:	4c 89 a5 00 ff ff ff 	mov    %r12,-0x100(%rbp)
    1701:	48 89 85 e8 fe ff ff 	mov    %rax,-0x118(%rbp)
    1708:	48 89 85 18 ff ff ff 	mov    %rax,-0xe8(%rbp)
    170f:	48 89 bd d0 fe ff ff 	mov    %rdi,-0x130(%rbp)
    1716:	48 8b b5 e0 fd ff ff 	mov    -0x220(%rbp),%rsi
    171d:	48 03 b5 00 ff ff ff 	add    -0x100(%rbp),%rsi
    1724:	6a 00                	push   $0x0
    1726:	45 31 c0             	xor    %r8d,%r8d
    1729:	48 2b b5 40 fe ff ff 	sub    -0x1c0(%rbp),%rsi
    1730:	4c 8b 8d 18 fe ff ff 	mov    -0x1e8(%rbp),%r9
    1737:	6a 00                	push   $0x0
    1739:	44 89 f2             	mov    %r14d,%edx
    173c:	48 8b 8d 20 fe ff ff 	mov    -0x1e0(%rbp),%rcx
    1743:	48 8b bd e8 fd ff ff 	mov    -0x218(%rbp),%rdi
    174a:	6a 08                	push   $0x8
    174c:	6a 08                	push   $0x8
    174e:	e8 fd f9 ff ff       	call   1150 <_gfortran_caf_get@plt>
    1753:	48 8b 85 78 fe ff ff 	mov    -0x188(%rbp),%rax
    175a:	48 83 c4 20          	add    $0x20,%rsp
    175e:	c5 fb 10 35 7a 1a 00 	vmovsd 0x1a7a(%rip),%xmm6        # 31e0 <options.97.4+0x50>
    1765:	00 
    1766:	c5 fb 10 50 08       	vmovsd 0x8(%rax),%xmm2
    176b:	c5 fb 10 18          	vmovsd (%rax),%xmm3
    176f:	c5 eb 5c 53 08       	vsubsd 0x8(%rbx),%xmm2,%xmm2
    1774:	c5 e3 5c 1b          	vsubsd (%rbx),%xmm3,%xmm3
    1778:	c5 fb 10 48 10       	vmovsd 0x10(%rax),%xmm1
    177d:	c5 f3 5c 4b 10       	vsubsd 0x10(%rbx),%xmm1,%xmm1
    1782:	c5 eb 59 c2          	vmulsd %xmm2,%xmm2,%xmm0
    1786:	c4 e2 e1 b9 c3       	vfmadd231sd %xmm3,%xmm3,%xmm0
    178b:	c4 e2 f1 b9 c1       	vfmadd231sd %xmm1,%xmm1,%xmm0
    1790:	c5 fb 51 e0          	vsqrtsd %xmm0,%xmm0,%xmm4
    1794:	c5 fb 59 c4          	vmulsd %xmm4,%xmm0,%xmm0
    1798:	c5 cb 5e c0          	vdivsd %xmm0,%xmm6,%xmm0
    179c:	c5 fb 59 85 88 fe ff 	vmulsd -0x178(%rbp),%xmm0,%xmm0
    17a3:	ff 
    17a4:	c5 e3 59 d8          	vmulsd %xmm0,%xmm3,%xmm3
    17a8:	c5 eb 59 d0          	vmulsd %xmm0,%xmm2,%xmm2
    17ac:	c5 f3 59 c0          	vmulsd %xmm0,%xmm1,%xmm0
    17b0:	48 39 85 30 ff ff ff 	cmp    %rax,-0xd0(%rbp)
    17b7:	74 38                	je     17f1 <__acceleratemodule_MOD_accelerateall+0x501>
    17b9:	48 89 c7             	mov    %rax,%rdi
    17bc:	c5 fb 11 85 00 fe ff 	vmovsd %xmm0,-0x200(%rbp)
    17c3:	ff 
    17c4:	c5 fb 11 95 08 fe ff 	vmovsd %xmm2,-0x1f8(%rbp)
    17cb:	ff 
    17cc:	c5 fb 11 9d 78 fe ff 	vmovsd %xmm3,-0x188(%rbp)
    17d3:	ff 
    17d4:	e8 97 f8 ff ff       	call   1070 <free@plt>
    17d9:	c5 fb 10 85 00 fe ff 	vmovsd -0x200(%rbp),%xmm0
    17e0:	ff 
    17e1:	c5 fb 10 95 08 fe ff 	vmovsd -0x1f8(%rbp),%xmm2
    17e8:	ff 
    17e9:	c5 fb 10 9d 78 fe ff 	vmovsd -0x188(%rbp),%xmm3
    17f0:	ff 
    17f1:	48 8b 85 60 fe ff ff 	mov    -0x1a0(%rbp),%rax
    17f8:	49 ff c5             	inc    %r13
    17fb:	49 83 c4 08          	add    $0x8,%r12
    17ff:	c5 e3 58 58 08       	vaddsd 0x8(%rax),%xmm3,%xmm3
    1804:	c5 fb 11 58 08       	vmovsd %xmm3,0x8(%rax)
    1809:	48 8b 85 68 fe ff ff 	mov    -0x198(%rbp),%rax
    1810:	c5 eb 58 50 08       	vaddsd 0x8(%rax),%xmm2,%xmm2
    1815:	c5 fb 11 50 08       	vmovsd %xmm2,0x8(%rax)
    181a:	48 8b 85 70 fe ff ff 	mov    -0x190(%rbp),%rax
    1821:	c5 fb 58 40 08       	vaddsd 0x8(%rax),%xmm0,%xmm0
    1826:	c5 fb 11 40 08       	vmovsd %xmm0,0x8(%rax)
    182b:	41 8d 45 01          	lea    0x1(%r13),%eax
    182f:	39 85 5c fe ff ff    	cmp    %eax,-0x1a4(%rbp)
    1835:	0f 8d b5 fd ff ff    	jge    15f0 <__acceleratemodule_MOD_accelerateall+0x300>
    183b:	48 ff 85 d8 fd ff ff 	incq   -0x228(%rbp)
    1842:	48 8b 85 d8 fd ff ff 	mov    -0x228(%rbp),%rax
    1849:	48 83 85 60 fe ff ff 	addq   $0x8,-0x1a0(%rbp)
    1850:	08 
    1851:	48 83 85 68 fe ff ff 	addq   $0x8,-0x198(%rbp)
    1858:	08 
    1859:	48 83 85 70 fe ff ff 	addq   $0x8,-0x190(%rbp)
    1860:	08 
    1861:	ff c0                	inc    %eax
    1863:	3b 85 d4 fd ff ff    	cmp    -0x22c(%rbp),%eax
    1869:	7f 58                	jg     18c3 <__acceleratemodule_MOD_accelerateall+0x5d3>
    186b:	48 8b 85 c8 fd ff ff 	mov    -0x238(%rbp),%rax
    1872:	8b 18                	mov    (%rax),%ebx
    1874:	e9 b7 fc ff ff       	jmp    1530 <__acceleratemodule_MOD_accelerateall+0x240>
    1879:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    1880:	48 8b 85 d8 fd ff ff 	mov    -0x228(%rbp),%rax
    1887:	48 8b bd c0 fd ff ff 	mov    -0x240(%rbp),%rdi
    188e:	48 8b 95 b8 fd ff ff 	mov    -0x248(%rbp),%rdx
    1895:	c5 fb 10 44 c7 08    	vmovsd 0x8(%rdi,%rax,8),%xmm0
    189b:	48 8b bd b0 fd ff ff 	mov    -0x250(%rbp),%rdi
    18a2:	c5 f9 16 44 c2 08    	vmovhpd 0x8(%rdx,%rax,8),%xmm0,%xmm0
    18a8:	c5 f9 29 85 90 fe ff 	vmovapd %xmm0,-0x170(%rbp)
    18af:	ff 
    18b0:	c5 fb 10 44 c7 08    	vmovsd 0x8(%rdi,%rax,8),%xmm0
    18b6:	c5 fb 11 85 a0 fe ff 	vmovsd %xmm0,-0x160(%rbp)
    18bd:	ff 
    18be:	e9 3a fd ff ff       	jmp    15fd <__acceleratemodule_MOD_accelerateall+0x30d>
    18c3:	41 ff c6             	inc    %r14d
    18c6:	44 39 b5 d0 fd ff ff 	cmp    %r14d,-0x230(%rbp)
    18cd:	7c 0e                	jl     18dd <__acceleratemodule_MOD_accelerateall+0x5ed>
    18cf:	48 8b 85 c8 fd ff ff 	mov    -0x238(%rbp),%rax
    18d6:	8b 18                	mov    (%rax),%ebx
    18d8:	e9 ed fb ff ff       	jmp    14ca <__acceleratemodule_MOD_accelerateall+0x1da>
    18dd:	48 8d 65 d0          	lea    -0x30(%rbp),%rsp
    18e1:	5b                   	pop    %rbx
    18e2:	41 5a                	pop    %r10
    18e4:	41 5c                	pop    %r12
    18e6:	41 5d                	pop    %r13
    18e8:	41 5e                	pop    %r14
    18ea:	41 5f                	pop    %r15
    18ec:	5d                   	pop    %rbp
    18ed:	49 8d 62 f8          	lea    -0x8(%r10),%rsp
    18f1:	c3                   	ret    
    18f2:	66 66 2e 0f 1f 84 00 	data16 cs nopw 0x0(%rax,%rax,1)
    18f9:	00 00 00 00 
    18fd:	0f 1f 00             	nopl   (%rax)

0000000000001900 <__acceleratemodule_MOD_accelerate>:
    1900:	48 89 f0             	mov    %rsi,%rax
    1903:	c5 fb 10 4a 08       	vmovsd 0x8(%rdx),%xmm1
    1908:	c5 fb 10 02          	vmovsd (%rdx),%xmm0
    190c:	49 89 c8             	mov    %rcx,%r8
    190f:	c5 f3 5c 48 08       	vsubsd 0x8(%rax),%xmm1,%xmm1
    1914:	c5 fb 5c 00          	vsubsd (%rax),%xmm0,%xmm0
    1918:	b9 01 00 00 00       	mov    $0x1,%ecx
    191d:	c5 fb 10 52 10       	vmovsd 0x10(%rdx),%xmm2
    1922:	48 8b 77 28          	mov    0x28(%rdi),%rsi
    1926:	c5 eb 5c 50 10       	vsubsd 0x10(%rax),%xmm2,%xmm2
    192b:	48 85 f6             	test   %rsi,%rsi
    192e:	48 0f 44 f1          	cmove  %rcx,%rsi
    1932:	48 8b 0f             	mov    (%rdi),%rcx
    1935:	c5 f3 59 c9          	vmulsd %xmm1,%xmm1,%xmm1
    1939:	c4 e2 f1 99 c0       	vfmadd132sd %xmm0,%xmm1,%xmm0
    193e:	c4 e2 e9 b9 c2       	vfmadd231sd %xmm2,%xmm2,%xmm0
    1943:	c5 fb 51 c8          	vsqrtsd %xmm0,%xmm0,%xmm1
    1947:	c5 f3 59 c8          	vmulsd %xmm0,%xmm1,%xmm1
    194b:	c4 c1 7b 10 00       	vmovsd (%r8),%xmm0
    1950:	c5 fb 5e c1          	vdivsd %xmm1,%xmm0,%xmm0
    1954:	c5 f9 10 0a          	vmovupd (%rdx),%xmm1
    1958:	c5 f1 5c 08          	vsubpd (%rax),%xmm1,%xmm1
    195c:	c5 fb 12 d8          	vmovddup %xmm0,%xmm3
    1960:	c5 f1 59 cb          	vmulpd %xmm3,%xmm1,%xmm1
    1964:	48 83 fe 01          	cmp    $0x1,%rsi
    1968:	75 16                	jne    1980 <__acceleratemodule_MOD_accelerate+0x80>
    196a:	c5 fb 59 c2          	vmulsd %xmm2,%xmm0,%xmm0
    196e:	c5 f9 11 09          	vmovupd %xmm1,(%rcx)
    1972:	c5 fb 11 41 10       	vmovsd %xmm0,0x10(%rcx)
    1977:	c3                   	ret    
    1978:	0f 1f 84 00 00 00 00 	nopl   0x0(%rax,%rax,1)
    197f:	00 
    1980:	c5 fb 59 c2          	vmulsd %xmm2,%xmm0,%xmm0
    1984:	c5 f9 13 09          	vmovlpd %xmm1,(%rcx)
    1988:	c5 f9 17 0c f1       	vmovhpd %xmm1,(%rcx,%rsi,8)
    198d:	48 01 f6             	add    %rsi,%rsi
    1990:	c5 fb 11 04 f1       	vmovsd %xmm0,(%rcx,%rsi,8)
    1995:	c3                   	ret    
    1996:	66 2e 0f 1f 84 00 00 	cs nopw 0x0(%rax,%rax,1)
    199d:	00 00 00 

00000000000019a0 <__nbodymodule_MOD_advanceit>:
    19a0:	41 55                	push   %r13
    19a2:	4c 8d 6c 24 10       	lea    0x10(%rsp),%r13
    19a7:	48 83 e4 e0          	and    $0xffffffffffffffe0,%rsp
    19ab:	41 ff 75 f8          	push   -0x8(%r13)
    19af:	55                   	push   %rbp
    19b0:	48 89 e5             	mov    %rsp,%rbp
    19b3:	41 57                	push   %r15
    19b5:	41 56                	push   %r14
    19b7:	41 55                	push   %r13
    19b9:	41 54                	push   %r12
    19bb:	53                   	push   %rbx
    19bc:	49 89 fe             	mov    %rdi,%r14
    19bf:	48 89 f3             	mov    %rsi,%rbx
    19c2:	48 83 ec 68          	sub    $0x68,%rsp
    19c6:	49 8b 75 20          	mov    0x20(%r13),%rsi
    19ca:	49 8b 7d 28          	mov    0x28(%r13),%rdi
    19ce:	48 89 95 78 ff ff ff 	mov    %rdx,-0x88(%rbp)
    19d5:	49 8b 45 00          	mov    0x0(%r13),%rax
    19d9:	49 89 cf             	mov    %rcx,%r15
    19dc:	4c 89 45 b8          	mov    %r8,-0x48(%rbp)
    19e0:	4c 89 c9             	mov    %r9,%rcx
    19e3:	4d 8b 45 30          	mov    0x30(%r13),%r8
    19e7:	48 89 4d 90          	mov    %rcx,-0x70(%rbp)
    19eb:	4d 8b 4d 38          	mov    0x38(%r13),%r9
    19ef:	49 8b 55 08          	mov    0x8(%r13),%rdx
    19f3:	44 8b 21             	mov    (%rcx),%r12d
    19f6:	48 89 75 a0          	mov    %rsi,-0x60(%rbp)
    19fa:	48 89 7d 98          	mov    %rdi,-0x68(%rbp)
    19fe:	be ff ff ff ff       	mov    $0xffffffff,%esi
    1a03:	31 ff                	xor    %edi,%edi
    1a05:	48 89 45 b0          	mov    %rax,-0x50(%rbp)
    1a09:	4c 89 45 80          	mov    %r8,-0x80(%rbp)
    1a0d:	4c 89 4d 88          	mov    %r9,-0x78(%rbp)
    1a11:	48 89 55 a8          	mov    %rdx,-0x58(%rbp)
    1a15:	e8 d6 f6 ff ff       	call   10f0 <_gfortran_caf_num_images@plt>
    1a1a:	be ff ff ff ff       	mov    $0xffffffff,%esi
    1a1f:	41 89 c3             	mov    %eax,%r11d
    1a22:	44 89 e0             	mov    %r12d,%eax
    1a25:	99                   	cltd   
    1a26:	41 f7 fb             	idiv   %r11d
    1a29:	4c 63 e8             	movslq %eax,%r13
    1a2c:	31 c0                	xor    %eax,%eax
    1a2e:	4d 85 ed             	test   %r13,%r13
    1a31:	4c 0f 48 e8          	cmovs  %rax,%r13
    1a35:	31 ff                	xor    %edi,%edi
    1a37:	e8 b4 f6 ff ff       	call   10f0 <_gfortran_caf_num_images@plt>
    1a3c:	41 89 c3             	mov    %eax,%r11d
    1a3f:	44 89 e0             	mov    %r12d,%eax
    1a42:	99                   	cltd   
    1a43:	41 f7 fb             	idiv   %r11d
    1a46:	48 63 f0             	movslq %eax,%rsi
    1a49:	31 c0                	xor    %eax,%eax
    1a4b:	48 85 f6             	test   %rsi,%rsi
    1a4e:	48 0f 49 c6          	cmovns %rsi,%rax
    1a52:	31 ff                	xor    %edi,%edi
    1a54:	be ff ff ff ff       	mov    $0xffffffff,%esi
    1a59:	48 89 45 c0          	mov    %rax,-0x40(%rbp)
    1a5d:	e8 8e f6 ff ff       	call   10f0 <_gfortran_caf_num_images@plt>
    1a62:	31 ff                	xor    %edi,%edi
    1a64:	be ff ff ff ff       	mov    $0xffffffff,%esi
    1a69:	e8 82 f6 ff ff       	call   10f0 <_gfortran_caf_num_images@plt>
    1a6e:	4c 89 f6             	mov    %r14,%rsi
    1a71:	4c 89 ff             	mov    %r15,%rdi
    1a74:	41 89 c3             	mov    %eax,%r11d
    1a77:	44 89 e0             	mov    %r12d,%eax
    1a7a:	ff 75 98             	push   -0x68(%rbp)
    1a7d:	4c 8b 4d 88          	mov    -0x78(%rbp),%r9
    1a81:	99                   	cltd   
    1a82:	ff 75 a0             	push   -0x60(%rbp)
    1a85:	41 f7 fb             	idiv   %r11d
    1a88:	ff 75 a8             	push   -0x58(%rbp)
    1a8b:	4c 8b 45 80          	mov    -0x80(%rbp),%r8
    1a8f:	ff 75 b0             	push   -0x50(%rbp)
    1a92:	48 8b 4d 90          	mov    -0x70(%rbp),%rcx
    1a96:	48 8b 95 78 ff ff ff 	mov    -0x88(%rbp),%rdx
    1a9d:	4c 63 e0             	movslq %eax,%r12
    1aa0:	31 c0                	xor    %eax,%eax
    1aa2:	4d 85 e4             	test   %r12,%r12
    1aa5:	49 0f 49 c4          	cmovns %r12,%rax
    1aa9:	48 89 45 c8          	mov    %rax,-0x38(%rbp)
    1aad:	e8 3e f8 ff ff       	call   12f0 <__acceleratemodule_MOD_accelerateall>
    1ab2:	48 8b 45 b8          	mov    -0x48(%rbp),%rax
    1ab6:	48 83 c4 20          	add    $0x20,%rsp
    1aba:	c5 fb 10 00          	vmovsd (%rax),%xmm0
    1abe:	4d 85 e4             	test   %r12,%r12
    1ac1:	0f 8e e0 01 00 00    	jle    1ca7 <__nbodymodule_MOD_advanceit+0x307>
    1ac7:	4d 89 e3             	mov    %r12,%r11
    1aca:	49 8d 44 24 ff       	lea    -0x1(%r12),%rax
    1acf:	4c 89 e6             	mov    %r12,%rsi
    1ad2:	48 c7 c1 ff ff ff ff 	mov    $0xffffffffffffffff,%rcx
    1ad9:	49 83 e3 fc          	and    $0xfffffffffffffffc,%r11
    1add:	48 89 45 b8          	mov    %rax,-0x48(%rbp)
    1ae1:	48 c1 ee 02          	shr    $0x2,%rsi
    1ae5:	48 c7 c2 ff ff ff ff 	mov    $0xffffffffffffffff,%rdx
    1aec:	49 8d 43 01          	lea    0x1(%r11),%rax
    1af0:	48 c1 e6 05          	shl    $0x5,%rsi
    1af4:	41 ba 03 00 00 00    	mov    $0x3,%r10d
    1afa:	c5 fb 12 d8          	vmovddup %xmm0,%xmm3
    1afe:	48 89 45 a0          	mov    %rax,-0x60(%rbp)
    1b02:	48 8d 43 08          	lea    0x8(%rbx),%rax
    1b06:	c4 e2 7d 19 d0       	vbroadcastsd %xmm0,%ymm2
    1b0b:	48 89 45 b0          	mov    %rax,-0x50(%rbp)
    1b0f:	49 8d 47 08          	lea    0x8(%r15),%rax
    1b13:	48 89 45 a8          	mov    %rax,-0x58(%rbp)
    1b17:	48 83 7d b8 02       	cmpq   $0x2,-0x48(%rbp)
    1b1c:	0f 86 a4 01 00 00    	jbe    1cc6 <__nbodymodule_MOD_advanceit+0x326>
    1b22:	48 8b 45 b0          	mov    -0x50(%rbp),%rax
    1b26:	48 8d 3c d0          	lea    (%rax,%rdx,8),%rdi
    1b2a:	48 8b 45 a8          	mov    -0x58(%rbp),%rax
    1b2e:	4c 8d 04 c8          	lea    (%rax,%rcx,8),%r8
    1b32:	31 c0                	xor    %eax,%eax
    1b34:	66 66 2e 0f 1f 84 00 	data16 cs nopw 0x0(%rax,%rax,1)
    1b3b:	00 00 00 00 
    1b3f:	90                   	nop
    1b40:	c4 c1 7d 10 0c 00    	vmovupd (%r8,%rax,1),%ymm1
    1b46:	c4 e2 ed a8 0c 07    	vfmadd213pd (%rdi,%rax,1),%ymm2,%ymm1
    1b4c:	c5 fd 11 0c 07       	vmovupd %ymm1,(%rdi,%rax,1)
    1b51:	48 83 c0 20          	add    $0x20,%rax
    1b55:	48 39 f0             	cmp    %rsi,%rax
    1b58:	75 e6                	jne    1b40 <__nbodymodule_MOD_advanceit+0x1a0>
    1b5a:	4d 39 dc             	cmp    %r11,%r12
    1b5d:	74 58                	je     1bb7 <__nbodymodule_MOD_advanceit+0x217>
    1b5f:	48 8b 45 a0          	mov    -0x60(%rbp),%rax
    1b63:	4d 89 d8             	mov    %r11,%r8
    1b66:	4c 89 e7             	mov    %r12,%rdi
    1b69:	4c 29 c7             	sub    %r8,%rdi
    1b6c:	48 83 ff 01          	cmp    $0x1,%rdi
    1b70:	74 2d                	je     1b9f <__nbodymodule_MOD_advanceit+0x1ff>
    1b72:	4d 8d 4c 10 01       	lea    0x1(%r8,%rdx,1),%r9
    1b77:	4d 8d 44 08 01       	lea    0x1(%r8,%rcx,1),%r8
    1b7c:	c4 81 79 10 0c c7    	vmovupd (%r15,%r8,8),%xmm1
    1b82:	4e 8d 0c cb          	lea    (%rbx,%r9,8),%r9
    1b86:	49 89 f8             	mov    %rdi,%r8
    1b89:	c4 c2 e1 a8 09       	vfmadd213pd (%r9),%xmm3,%xmm1
    1b8e:	49 83 e0 fe          	and    $0xfffffffffffffffe,%r8
    1b92:	4c 01 c0             	add    %r8,%rax
    1b95:	c4 c1 79 11 09       	vmovupd %xmm1,(%r9)
    1b9a:	4c 39 c7             	cmp    %r8,%rdi
    1b9d:	74 18                	je     1bb7 <__nbodymodule_MOD_advanceit+0x217>
    1b9f:	48 8d 3c 02          	lea    (%rdx,%rax,1),%rdi
    1ba3:	48 01 c8             	add    %rcx,%rax
    1ba6:	c4 c1 7b 10 0c c7    	vmovsd (%r15,%rax,8),%xmm1
    1bac:	c4 e2 f9 a9 0c fb    	vfmadd213sd (%rbx,%rdi,8),%xmm0,%xmm1
    1bb2:	c5 fb 11 0c fb       	vmovsd %xmm1,(%rbx,%rdi,8)
    1bb7:	48 03 55 c8          	add    -0x38(%rbp),%rdx
    1bbb:	4c 01 e9             	add    %r13,%rcx
    1bbe:	49 ff ca             	dec    %r10
    1bc1:	0f 85 50 ff ff ff    	jne    1b17 <__nbodymodule_MOD_advanceit+0x177>
    1bc7:	4d 89 e3             	mov    %r12,%r11
    1bca:	49 8d 44 24 ff       	lea    -0x1(%r12),%rax
    1bcf:	4c 89 e6             	mov    %r12,%rsi
    1bd2:	48 c7 c1 ff ff ff ff 	mov    $0xffffffffffffffff,%rcx
    1bd9:	49 83 e3 fc          	and    $0xfffffffffffffffc,%r11
    1bdd:	48 89 45 b8          	mov    %rax,-0x48(%rbp)
    1be1:	48 c1 ee 02          	shr    $0x2,%rsi
    1be5:	48 c7 c2 ff ff ff ff 	mov    $0xffffffffffffffff,%rdx
    1bec:	49 8d 43 01          	lea    0x1(%r11),%rax
    1bf0:	48 c1 e6 05          	shl    $0x5,%rsi
    1bf4:	41 b9 03 00 00 00    	mov    $0x3,%r9d
    1bfa:	c5 fb 12 d8          	vmovddup %xmm0,%xmm3
    1bfe:	48 89 45 b0          	mov    %rax,-0x50(%rbp)
    1c02:	4c 8d 7b 08          	lea    0x8(%rbx),%r15
    1c06:	c4 e2 7d 19 d0       	vbroadcastsd %xmm0,%ymm2
    1c0b:	4d 8d 6e 08          	lea    0x8(%r14),%r13
    1c0f:	48 83 7d b8 02       	cmpq   $0x2,-0x48(%rbp)
    1c14:	0f 86 b9 00 00 00    	jbe    1cd3 <__nbodymodule_MOD_advanceit+0x333>
    1c1a:	4d 8d 04 d7          	lea    (%r15,%rdx,8),%r8
    1c1e:	49 8d 7c cd 00       	lea    0x0(%r13,%rcx,8),%rdi
    1c23:	31 c0                	xor    %eax,%eax
    1c25:	66 66 2e 0f 1f 84 00 	data16 cs nopw 0x0(%rax,%rax,1)
    1c2c:	00 00 00 00 
    1c30:	c4 c1 6d 59 0c 00    	vmulpd (%r8,%rax,1),%ymm2,%ymm1
    1c36:	c5 fd 11 0c 07       	vmovupd %ymm1,(%rdi,%rax,1)
    1c3b:	48 83 c0 20          	add    $0x20,%rax
    1c3f:	48 39 c6             	cmp    %rax,%rsi
    1c42:	75 ec                	jne    1c30 <__nbodymodule_MOD_advanceit+0x290>
    1c44:	4d 39 e3             	cmp    %r12,%r11
    1c47:	74 4a                	je     1c93 <__nbodymodule_MOD_advanceit+0x2f3>
    1c49:	48 8b 45 b0          	mov    -0x50(%rbp),%rax
    1c4d:	4d 89 d8             	mov    %r11,%r8
    1c50:	4c 89 e7             	mov    %r12,%rdi
    1c53:	4c 29 c7             	sub    %r8,%rdi
    1c56:	48 83 ff 01          	cmp    $0x1,%rdi
    1c5a:	74 25                	je     1c81 <__nbodymodule_MOD_advanceit+0x2e1>
    1c5c:	4d 8d 54 10 01       	lea    0x1(%r8,%rdx,1),%r10
    1c61:	4d 8d 44 08 01       	lea    0x1(%r8,%rcx,1),%r8
    1c66:	c4 a1 61 59 0c d3    	vmulpd (%rbx,%r10,8),%xmm3,%xmm1
    1c6c:	c4 81 79 11 0c c6    	vmovupd %xmm1,(%r14,%r8,8)
    1c72:	49 89 f8             	mov    %rdi,%r8
    1c75:	49 83 e0 fe          	and    $0xfffffffffffffffe,%r8
    1c79:	4c 01 c0             	add    %r8,%rax
    1c7c:	4c 39 c7             	cmp    %r8,%rdi
    1c7f:	74 12                	je     1c93 <__nbodymodule_MOD_advanceit+0x2f3>
    1c81:	48 8d 3c 01          	lea    (%rcx,%rax,1),%rdi
    1c85:	48 01 d0             	add    %rdx,%rax
    1c88:	c5 fb 59 0c c3       	vmulsd (%rbx,%rax,8),%xmm0,%xmm1
    1c8d:	c4 c1 7b 11 0c fe    	vmovsd %xmm1,(%r14,%rdi,8)
    1c93:	48 03 55 c8          	add    -0x38(%rbp),%rdx
    1c97:	48 03 4d c0          	add    -0x40(%rbp),%rcx
    1c9b:	49 ff c9             	dec    %r9
    1c9e:	0f 85 6b ff ff ff    	jne    1c0f <__nbodymodule_MOD_advanceit+0x26f>
    1ca4:	c5 f8 77             	vzeroupper 
    1ca7:	48 8d 65 d8          	lea    -0x28(%rbp),%rsp
    1cab:	31 d2                	xor    %edx,%edx
    1cad:	31 f6                	xor    %esi,%esi
    1caf:	31 ff                	xor    %edi,%edi
    1cb1:	5b                   	pop    %rbx
    1cb2:	41 5c                	pop    %r12
    1cb4:	41 5d                	pop    %r13
    1cb6:	41 5e                	pop    %r14
    1cb8:	41 5f                	pop    %r15
    1cba:	5d                   	pop    %rbp
    1cbb:	49 8d 65 f0          	lea    -0x10(%r13),%rsp
    1cbf:	41 5d                	pop    %r13
    1cc1:	e9 ea f3 ff ff       	jmp    10b0 <_gfortran_caf_sync_all@plt>
    1cc6:	45 31 c0             	xor    %r8d,%r8d
    1cc9:	b8 01 00 00 00       	mov    $0x1,%eax
    1cce:	e9 93 fe ff ff       	jmp    1b66 <__nbodymodule_MOD_advanceit+0x1c6>
    1cd3:	45 31 c0             	xor    %r8d,%r8d
    1cd6:	b8 01 00 00 00       	mov    $0x1,%eax
    1cdb:	e9 70 ff ff ff       	jmp    1c50 <__nbodymodule_MOD_advanceit+0x2b0>

0000000000001ce0 <MAIN__>:
    1ce0:	55                   	push   %rbp
    1ce1:	48 b8 00 00 00 00 02 	movabs $0x30200000000,%rax
    1ce8:	03 00 00 
    1ceb:	bf 18 00 00 00       	mov    $0x18,%edi
    1cf0:	48 89 e5             	mov    %rsp,%rbp
    1cf3:	41 57                	push   %r15
    1cf5:	41 56                	push   %r14
    1cf7:	41 55                	push   %r13
    1cf9:	41 54                	push   %r12
    1cfb:	41 52                	push   %r10
    1cfd:	53                   	push   %rbx
    1cfe:	48 bb 00 00 00 00 01 	movabs $0x30100000000,%rbx
    1d05:	03 00 00 
    1d08:	48 81 ec 50 02 00 00 	sub    $0x250,%rsp
    1d0f:	48 c7 05 86 34 00 00 	movq   $0x0,0x3486(%rip)        # 51a0 <accel.3>
    1d16:	00 00 00 00 
    1d1a:	48 c7 05 eb 34 00 00 	movq   $0x0,0x34eb(%rip)        # 5210 <accel.3+0x70>
    1d21:	00 00 00 00 
    1d25:	48 c7 05 80 34 00 00 	movq   $0x8,0x3480(%rip)        # 51b0 <accel.3+0x10>
    1d2c:	08 00 00 00 
    1d30:	48 89 05 81 34 00 00 	mov    %rax,0x3481(%rip)        # 51b8 <accel.3+0x18>
    1d37:	48 c7 05 fe 33 00 00 	movq   $0x0,0x33fe(%rip)        # 5140 <masses.2>
    1d3e:	00 00 00 00 
    1d42:	48 c7 05 4b 34 00 00 	movq   $0x0,0x344b(%rip)        # 5198 <masses.2+0x58>
    1d49:	00 00 00 00 
    1d4d:	48 c7 05 f8 33 00 00 	movq   $0x8,0x33f8(%rip)        # 5150 <masses.2+0x10>
    1d54:	08 00 00 00 
    1d58:	48 89 1d f9 33 00 00 	mov    %rbx,0x33f9(%rip)        # 5158 <masses.2+0x18>
    1d5f:	48 c7 05 56 33 00 00 	movq   $0x0,0x3356(%rip)        # 50c0 <positions.1>
    1d66:	00 00 00 00 
    1d6a:	48 c7 05 bb 33 00 00 	movq   $0x0,0x33bb(%rip)        # 5130 <positions.1+0x70>
    1d71:	00 00 00 00 
    1d75:	48 c7 05 50 33 00 00 	movq   $0x8,0x3350(%rip)        # 50d0 <positions.1+0x10>
    1d7c:	08 00 00 00 
    1d80:	48 89 05 51 33 00 00 	mov    %rax,0x3351(%rip)        # 50d8 <positions.1+0x18>
    1d87:	48 c7 05 ae 32 00 00 	movq   $0x0,0x32ae(%rip)        # 5040 <velocities.0>
    1d8e:	00 00 00 00 
    1d92:	48 c7 05 13 33 00 00 	movq   $0x0,0x3313(%rip)        # 50b0 <velocities.0+0x70>
    1d99:	00 00 00 00 
    1d9d:	48 c7 05 a8 32 00 00 	movq   $0x8,0x32a8(%rip)        # 5050 <velocities.0+0x10>
    1da4:	08 00 00 00 
    1da8:	48 89 05 a9 32 00 00 	mov    %rax,0x32a9(%rip)        # 5058 <velocities.0+0x18>
    1daf:	e8 ac f2 ff ff       	call   1060 <malloc@plt>
    1db4:	48 85 c0             	test   %rax,%rax
    1db7:	0f 84 86 08 00 00    	je     2643 <MAIN__+0x963>
    1dbd:	41 b8 0c 00 00 00    	mov    $0xc,%r8d
    1dc3:	31 c9                	xor    %ecx,%ecx
    1dc5:	48 89 c6             	mov    %rax,%rsi
    1dc8:	49 89 c4             	mov    %rax,%r12
    1dcb:	31 d2                	xor    %edx,%edx
    1dcd:	31 c0                	xor    %eax,%eax
    1dcf:	48 8d 3d aa 13 00 00 	lea    0x13aa(%rip),%rdi        # 3180 <_IO_stdin_used+0x180>
    1dd6:	48 8d 9d c0 fd ff ff 	lea    -0x240(%rbp),%rbx
    1ddd:	e8 3e f3 ff ff       	call   1120 <_gfortran_get_command_argument_i4@plt>
    1de2:	4c 8b 3d 17 14 00 00 	mov    0x1417(%rip),%r15        # 3200 <options.97.4+0x70>
    1de9:	4c 8d 35 2f 12 00 00 	lea    0x122f(%rip),%r14        # 301f <_IO_stdin_used+0x1f>
    1df0:	48 89 df             	mov    %rbx,%rdi
    1df3:	4c 8d ad bc fd ff ff 	lea    -0x244(%rbp),%r13
    1dfa:	4c 89 a5 30 fe ff ff 	mov    %r12,-0x1d0(%rbp)
    1e01:	4c 89 b5 c8 fd ff ff 	mov    %r14,-0x238(%rbp)
    1e08:	c7 85 d0 fd ff ff 5c 	movl   $0x5c,-0x230(%rbp)
    1e0f:	00 00 00 
    1e12:	48 c7 85 38 fe ff ff 	movq   $0xc,-0x1c8(%rbp)
    1e19:	0c 00 00 00 
    1e1d:	48 c7 85 08 fe ff ff 	movq   $0x0,-0x1f8(%rbp)
    1e24:	00 00 00 00 
    1e28:	49 83 c4 0c          	add    $0xc,%r12
    1e2c:	4c 89 bd c0 fd ff ff 	mov    %r15,-0x240(%rbp)
    1e33:	e8 88 f2 ff ff       	call   10c0 <_gfortran_st_read@plt>
    1e38:	ba 04 00 00 00       	mov    $0x4,%edx
    1e3d:	4c 89 ee             	mov    %r13,%rsi
    1e40:	48 89 df             	mov    %rbx,%rdi
    1e43:	e8 38 f2 ff ff       	call   1080 <_gfortran_transfer_integer@plt>
    1e48:	48 89 df             	mov    %rbx,%rdi
    1e4b:	e8 00 f2 ff ff       	call   1050 <_gfortran_st_read_done@plt>
    1e50:	41 b8 0c 00 00 00    	mov    $0xc,%r8d
    1e56:	31 c9                	xor    %ecx,%ecx
    1e58:	31 c0                	xor    %eax,%eax
    1e5a:	4c 89 e6             	mov    %r12,%rsi
    1e5d:	31 d2                	xor    %edx,%edx
    1e5f:	48 8d 3d 1e 13 00 00 	lea    0x131e(%rip),%rdi        # 3184 <_IO_stdin_used+0x184>
    1e66:	e8 b5 f2 ff ff       	call   1120 <_gfortran_get_command_argument_i4@plt>
    1e6b:	48 89 df             	mov    %rbx,%rdi
    1e6e:	4c 89 a5 30 fe ff ff 	mov    %r12,-0x1d0(%rbp)
    1e75:	4c 89 bd c0 fd ff ff 	mov    %r15,-0x240(%rbp)
    1e7c:	4c 89 b5 c8 fd ff ff 	mov    %r14,-0x238(%rbp)
    1e83:	c7 85 d0 fd ff ff 5f 	movl   $0x5f,-0x230(%rbp)
    1e8a:	00 00 00 
    1e8d:	48 c7 85 38 fe ff ff 	movq   $0xc,-0x1c8(%rbp)
    1e94:	0c 00 00 00 
    1e98:	48 c7 85 08 fe ff ff 	movq   $0x0,-0x1f8(%rbp)
    1e9f:	00 00 00 00 
    1ea3:	e8 18 f2 ff ff       	call   10c0 <_gfortran_st_read@plt>
    1ea8:	ba 04 00 00 00       	mov    $0x4,%edx
    1ead:	48 8d b5 b8 fd ff ff 	lea    -0x248(%rbp),%rsi
    1eb4:	48 89 df             	mov    %rbx,%rdi
    1eb7:	e8 c4 f1 ff ff       	call   1080 <_gfortran_transfer_integer@plt>
    1ebc:	48 89 df             	mov    %rbx,%rdi
    1ebf:	e8 8c f1 ff ff       	call   1050 <_gfortran_st_read_done@plt>
    1ec4:	be ff ff ff ff       	mov    $0xffffffff,%esi
    1ec9:	31 ff                	xor    %edi,%edi
    1ecb:	e8 20 f2 ff ff       	call   10f0 <_gfortran_caf_num_images@plt>
    1ed0:	44 8b bd bc fd ff ff 	mov    -0x244(%rbp),%r15d
    1ed7:	41 89 c4             	mov    %eax,%r12d
    1eda:	44 89 f8             	mov    %r15d,%eax
    1edd:	99                   	cltd   
    1ede:	41 f7 fc             	idiv   %r12d
    1ee1:	85 d2                	test   %edx,%edx
    1ee3:	0f 85 f7 06 00 00    	jne    25e0 <MAIN__+0x900>
    1ee9:	48 b8 00 00 00 00 02 	movabs $0x30200000000,%rax
    1ef0:	03 00 00 
    1ef3:	48 c7 05 d2 31 00 00 	movq   $0x8,0x31d2(%rip)        # 50d0 <positions.1+0x10>
    1efa:	08 00 00 00 
    1efe:	48 89 05 d3 31 00 00 	mov    %rax,0x31d3(%rip)        # 50d8 <positions.1+0x18>
    1f05:	44 89 f8             	mov    %r15d,%eax
    1f08:	45 31 ff             	xor    %r15d,%r15d
    1f0b:	99                   	cltd   
    1f0c:	41 f7 fc             	idiv   %r12d
    1f0f:	ba 00 00 00 00       	mov    $0x0,%edx
    1f14:	85 c0                	test   %eax,%eax
    1f16:	44 0f 49 f8          	cmovns %eax,%r15d
    1f1a:	49 63 cf             	movslq %r15d,%rcx
    1f1d:	49 89 ce             	mov    %rcx,%r14
    1f20:	49 f7 d6             	not    %r14
    1f23:	7e 0c                	jle    1f31 <MAIN__+0x251>
    1f25:	48 8d 0c 49          	lea    (%rcx,%rcx,2),%rcx
    1f29:	48 8d 14 cd 00 00 00 	lea    0x0(,%rcx,8),%rdx
    1f30:	00 
    1f31:	48 83 3d 87 31 00 00 	cmpq   $0x0,0x3187(%rip)        # 50c0 <positions.1>
    1f38:	00 
    1f39:	0f 85 72 07 00 00    	jne    26b1 <MAIN__+0x9d1>
    1f3f:	48 85 d2             	test   %rdx,%rdx
    1f42:	b8 01 00 00 00       	mov    $0x1,%eax
    1f47:	48 8d 0d 72 31 00 00 	lea    0x3172(%rip),%rcx        # 50c0 <positions.1>
    1f4e:	be 01 00 00 00       	mov    $0x1,%esi
    1f53:	48 0f 45 c2          	cmovne %rdx,%rax
    1f57:	48 83 ec 08          	sub    $0x8,%rsp
    1f5b:	45 31 c9             	xor    %r9d,%r9d
    1f5e:	45 31 c0             	xor    %r8d,%r8d
    1f61:	6a 00                	push   $0x0
    1f63:	48 89 c7             	mov    %rax,%rdi
    1f66:	48 8d 51 70          	lea    0x70(%rcx),%rdx
    1f6a:	e8 a1 f1 ff ff       	call   1110 <_gfortran_caf_register@plt>
    1f6f:	8b 85 bc fd ff ff    	mov    -0x244(%rbp),%eax
    1f75:	c5 f9 6f 15 a3 12 00 	vmovdqa 0x12a3(%rip),%xmm2        # 3220 <options.97.4+0x90>
    1f7c:	00 
    1f7d:	31 f6                	xor    %esi,%esi
    1f7f:	31 ff                	xor    %edi,%edi
    1f81:	c5 f9 6f 25 37 12 00 	vmovdqa 0x1237(%rip),%xmm4        # 31c0 <options.97.4+0x30>
    1f88:	00 
    1f89:	4c 89 35 38 31 00 00 	mov    %r14,0x3138(%rip)        # 50c8 <positions.1+0x8>
    1f90:	48 c7 05 55 31 00 00 	movq   $0x1,0x3155(%rip)        # 50f0 <positions.1+0x30>
    1f97:	01 00 00 00 
    1f9b:	48 c7 05 7a 31 00 00 	movq   $0x1,0x317a(%rip)        # 5120 <positions.1+0x60>
    1fa2:	01 00 00 00 
    1fa6:	99                   	cltd   
    1fa7:	41 f7 fc             	idiv   %r12d
    1faa:	31 d2                	xor    %edx,%edx
    1fac:	c5 fa 7f 15 54 31 00 	vmovdqu %xmm2,0x3154(%rip)        # 5108 <positions.1+0x48>
    1fb3:	00 
    1fb4:	c5 f9 7f 25 24 31 00 	vmovdqa %xmm4,0x3124(%rip)        # 50e0 <positions.1+0x20>
    1fbb:	00 
    1fbc:	c5 f9 6e d8          	vmovd  %eax,%xmm3
    1fc0:	89 85 a8 fd ff ff    	mov    %eax,-0x258(%rbp)
    1fc6:	c4 c3 61 22 c7 01    	vpinsrd $0x1,%r15d,%xmm3,%xmm0
    1fcc:	45 31 ff             	xor    %r15d,%r15d
    1fcf:	c4 e2 79 25 c0       	vpmovsxdq %xmm0,%xmm0
    1fd4:	c5 fa 7f 05 1c 31 00 	vmovdqu %xmm0,0x311c(%rip)        # 50f8 <positions.1+0x38>
    1fdb:	00 
    1fdc:	e8 cf f0 ff ff       	call   10b0 <_gfortran_caf_sync_all@plt>
    1fe1:	48 b8 00 00 00 00 02 	movabs $0x30200000000,%rax
    1fe8:	03 00 00 
    1feb:	5e                   	pop    %rsi
    1fec:	48 89 05 65 30 00 00 	mov    %rax,0x3065(%rip)        # 5058 <velocities.0+0x18>
    1ff3:	8b 85 a8 fd ff ff    	mov    -0x258(%rbp),%eax
    1ff9:	48 c7 05 4c 30 00 00 	movq   $0x8,0x304c(%rip)        # 5050 <velocities.0+0x10>
    2000:	08 00 00 00 
    2004:	5f                   	pop    %rdi
    2005:	85 c0                	test   %eax,%eax
    2007:	44 0f 49 f8          	cmovns %eax,%r15d
    200b:	31 d2                	xor    %edx,%edx
    200d:	49 63 cf             	movslq %r15d,%rcx
    2010:	49 89 ce             	mov    %rcx,%r14
    2013:	49 f7 d6             	not    %r14
    2016:	85 c0                	test   %eax,%eax
    2018:	7e 0c                	jle    2026 <MAIN__+0x346>
    201a:	48 8d 0c 49          	lea    (%rcx,%rcx,2),%rcx
    201e:	48 8d 14 cd 00 00 00 	lea    0x0(,%rcx,8),%rdx
    2025:	00 
    2026:	48 83 3d 12 30 00 00 	cmpq   $0x0,0x3012(%rip)        # 5040 <velocities.0>
    202d:	00 
    202e:	0f 85 61 06 00 00    	jne    2695 <MAIN__+0x9b5>
    2034:	48 85 d2             	test   %rdx,%rdx
    2037:	b8 01 00 00 00       	mov    $0x1,%eax
    203c:	48 8d 0d fd 2f 00 00 	lea    0x2ffd(%rip),%rcx        # 5040 <velocities.0>
    2043:	be 01 00 00 00       	mov    $0x1,%esi
    2048:	48 0f 45 c2          	cmovne %rdx,%rax
    204c:	48 83 ec 08          	sub    $0x8,%rsp
    2050:	45 31 c9             	xor    %r9d,%r9d
    2053:	45 31 c0             	xor    %r8d,%r8d
    2056:	6a 00                	push   $0x0
    2058:	48 89 c7             	mov    %rax,%rdi
    205b:	48 8d 51 70          	lea    0x70(%rcx),%rdx
    205f:	e8 ac f0 ff ff       	call   1110 <_gfortran_caf_register@plt>
    2064:	8b 85 bc fd ff ff    	mov    -0x244(%rbp),%eax
    206a:	c5 f9 6f 2d ae 11 00 	vmovdqa 0x11ae(%rip),%xmm5        # 3220 <options.97.4+0x90>
    2071:	00 
    2072:	31 f6                	xor    %esi,%esi
    2074:	31 ff                	xor    %edi,%edi
    2076:	c5 f9 6f 3d 42 11 00 	vmovdqa 0x1142(%rip),%xmm7        # 31c0 <options.97.4+0x30>
    207d:	00 
    207e:	4c 89 35 c3 2f 00 00 	mov    %r14,0x2fc3(%rip)        # 5048 <velocities.0+0x8>
    2085:	48 c7 05 e0 2f 00 00 	movq   $0x1,0x2fe0(%rip)        # 5070 <velocities.0+0x30>
    208c:	01 00 00 00 
    2090:	48 c7 05 05 30 00 00 	movq   $0x1,0x3005(%rip)        # 50a0 <velocities.0+0x60>
    2097:	01 00 00 00 
    209b:	99                   	cltd   
    209c:	41 f7 fc             	idiv   %r12d
    209f:	31 d2                	xor    %edx,%edx
    20a1:	c5 fa 7f 2d df 2f 00 	vmovdqu %xmm5,0x2fdf(%rip)        # 5088 <velocities.0+0x48>
    20a8:	00 
    20a9:	c5 f9 7f 3d af 2f 00 	vmovdqa %xmm7,0x2faf(%rip)        # 5060 <velocities.0+0x20>
    20b0:	00 
    20b1:	c5 f9 6e f0          	vmovd  %eax,%xmm6
    20b5:	89 85 a8 fd ff ff    	mov    %eax,-0x258(%rbp)
    20bb:	c4 c3 49 22 c7 01    	vpinsrd $0x1,%r15d,%xmm6,%xmm0
    20c1:	45 31 ff             	xor    %r15d,%r15d
    20c4:	c4 e2 79 25 c0       	vpmovsxdq %xmm0,%xmm0
    20c9:	c5 fa 7f 05 a7 2f 00 	vmovdqu %xmm0,0x2fa7(%rip)        # 5078 <velocities.0+0x38>
    20d0:	00 
    20d1:	e8 da ef ff ff       	call   10b0 <_gfortran_caf_sync_all@plt>
    20d6:	48 b8 00 00 00 00 02 	movabs $0x30200000000,%rax
    20dd:	03 00 00 
    20e0:	41 5b                	pop    %r11
    20e2:	48 89 05 cf 30 00 00 	mov    %rax,0x30cf(%rip)        # 51b8 <accel.3+0x18>
    20e9:	8b 85 a8 fd ff ff    	mov    -0x258(%rbp),%eax
    20ef:	48 c7 05 b6 30 00 00 	movq   $0x8,0x30b6(%rip)        # 51b0 <accel.3+0x10>
    20f6:	08 00 00 00 
    20fa:	5a                   	pop    %rdx
    20fb:	85 c0                	test   %eax,%eax
    20fd:	44 0f 49 f8          	cmovns %eax,%r15d
    2101:	31 d2                	xor    %edx,%edx
    2103:	49 63 cf             	movslq %r15d,%rcx
    2106:	49 89 ce             	mov    %rcx,%r14
    2109:	49 f7 d6             	not    %r14
    210c:	85 c0                	test   %eax,%eax
    210e:	7e 0c                	jle    211c <MAIN__+0x43c>
    2110:	48 8d 0c 49          	lea    (%rcx,%rcx,2),%rcx
    2114:	48 8d 14 cd 00 00 00 	lea    0x0(,%rcx,8),%rdx
    211b:	00 
    211c:	48 83 3d 7c 30 00 00 	cmpq   $0x0,0x307c(%rip)        # 51a0 <accel.3>
    2123:	00 
    2124:	0f 85 4f 05 00 00    	jne    2679 <MAIN__+0x999>
    212a:	48 85 d2             	test   %rdx,%rdx
    212d:	b8 01 00 00 00       	mov    $0x1,%eax
    2132:	48 8d 0d 67 30 00 00 	lea    0x3067(%rip),%rcx        # 51a0 <accel.3>
    2139:	be 01 00 00 00       	mov    $0x1,%esi
    213e:	48 0f 45 c2          	cmovne %rdx,%rax
    2142:	48 83 ec 08          	sub    $0x8,%rsp
    2146:	45 31 c9             	xor    %r9d,%r9d
    2149:	45 31 c0             	xor    %r8d,%r8d
    214c:	6a 00                	push   $0x0
    214e:	48 89 c7             	mov    %rax,%rdi
    2151:	48 8d 51 70          	lea    0x70(%rcx),%rdx
    2155:	e8 b6 ef ff ff       	call   1110 <_gfortran_caf_register@plt>
    215a:	8b 85 bc fd ff ff    	mov    -0x244(%rbp),%eax
    2160:	c5 f9 6f 15 b8 10 00 	vmovdqa 0x10b8(%rip),%xmm2        # 3220 <options.97.4+0x90>
    2167:	00 
    2168:	31 f6                	xor    %esi,%esi
    216a:	31 ff                	xor    %edi,%edi
    216c:	c5 f9 6f 25 4c 10 00 	vmovdqa 0x104c(%rip),%xmm4        # 31c0 <options.97.4+0x30>
    2173:	00 
    2174:	48 c7 05 51 30 00 00 	movq   $0x1,0x3051(%rip)        # 51d0 <accel.3+0x30>
    217b:	01 00 00 00 
    217f:	48 c7 05 76 30 00 00 	movq   $0x1,0x3076(%rip)        # 5200 <accel.3+0x60>
    2186:	01 00 00 00 
    218a:	4c 89 35 17 30 00 00 	mov    %r14,0x3017(%rip)        # 51a8 <accel.3+0x8>
    2191:	99                   	cltd   
    2192:	41 f7 fc             	idiv   %r12d
    2195:	31 d2                	xor    %edx,%edx
    2197:	c5 fa 7f 15 49 30 00 	vmovdqu %xmm2,0x3049(%rip)        # 51e8 <accel.3+0x48>
    219e:	00 
    219f:	c5 f9 7f 25 19 30 00 	vmovdqa %xmm4,0x3019(%rip)        # 51c0 <accel.3+0x20>
    21a6:	00 
    21a7:	c5 f9 6e d8          	vmovd  %eax,%xmm3
    21ab:	89 85 a8 fd ff ff    	mov    %eax,-0x258(%rbp)
    21b1:	c4 c3 61 22 c7 01    	vpinsrd $0x1,%r15d,%xmm3,%xmm0
    21b7:	c4 e2 79 25 c0       	vpmovsxdq %xmm0,%xmm0
    21bc:	c5 fa 7f 05 14 30 00 	vmovdqu %xmm0,0x3014(%rip)        # 51d8 <accel.3+0x38>
    21c3:	00 
    21c4:	e8 e7 ee ff ff       	call   10b0 <_gfortran_caf_sync_all@plt>
    21c9:	48 63 95 a8 fd ff ff 	movslq -0x258(%rbp),%rdx
    21d0:	48 b8 00 00 00 00 01 	movabs $0x30100000000,%rax
    21d7:	03 00 00 
    21da:	48 89 05 77 2f 00 00 	mov    %rax,0x2f77(%rip)        # 5158 <masses.2+0x18>
    21e1:	41 59                	pop    %r9
    21e3:	48 c7 05 62 2f 00 00 	movq   $0x8,0x2f62(%rip)        # 5150 <masses.2+0x10>
    21ea:	08 00 00 00 
    21ee:	41 5a                	pop    %r10
    21f0:	48 89 d0             	mov    %rdx,%rax
    21f3:	48 c1 e2 03          	shl    $0x3,%rdx
    21f7:	85 c0                	test   %eax,%eax
    21f9:	b8 00 00 00 00       	mov    $0x0,%eax
    21fe:	48 0f 4f c2          	cmovg  %rdx,%rax
    2202:	48 83 3d 36 2f 00 00 	cmpq   $0x0,0x2f36(%rip)        # 5140 <masses.2>
    2209:	00 
    220a:	0f 85 4d 04 00 00    	jne    265d <MAIN__+0x97d>
    2210:	48 85 c0             	test   %rax,%rax
    2213:	bf 01 00 00 00       	mov    $0x1,%edi
    2218:	48 8d 0d 21 2f 00 00 	lea    0x2f21(%rip),%rcx        # 5140 <masses.2>
    221f:	be 01 00 00 00       	mov    $0x1,%esi
    2224:	48 0f 45 f8          	cmovne %rax,%rdi
    2228:	48 83 ec 08          	sub    $0x8,%rsp
    222c:	45 31 c9             	xor    %r9d,%r9d
    222f:	45 31 c0             	xor    %r8d,%r8d
    2232:	6a 00                	push   $0x0
    2234:	48 8d 51 58          	lea    0x58(%rcx),%rdx
    2238:	e8 d3 ee ff ff       	call   1110 <_gfortran_caf_register@plt>
    223d:	8b 85 bc fd ff ff    	mov    -0x244(%rbp),%eax
    2243:	c5 f9 6f 2d 75 0f 00 	vmovdqa 0xf75(%rip),%xmm5        # 31c0 <options.97.4+0x30>
    224a:	00 
    224b:	31 f6                	xor    %esi,%esi
    224d:	31 ff                	xor    %edi,%edi
    224f:	48 c7 05 16 2f 00 00 	movq   $0x1,0x2f16(%rip)        # 5170 <masses.2+0x30>
    2256:	01 00 00 00 
    225a:	48 c7 05 23 2f 00 00 	movq   $0x1,0x2f23(%rip)        # 5188 <masses.2+0x48>
    2261:	01 00 00 00 
    2265:	48 c7 05 d8 2e 00 00 	movq   $0xffffffffffffffff,0x2ed8(%rip)        # 5148 <masses.2+0x8>
    226c:	ff ff ff ff 
    2270:	99                   	cltd   
    2271:	41 f7 fc             	idiv   %r12d
    2274:	31 d2                	xor    %edx,%edx
    2276:	c5 f9 7f 2d e2 2e 00 	vmovdqa %xmm5,0x2ee2(%rip)        # 5160 <masses.2+0x20>
    227d:	00 
    227e:	48 98                	cltq   
    2280:	48 89 05 f1 2e 00 00 	mov    %rax,0x2ef1(%rip)        # 5178 <masses.2+0x38>
    2287:	e8 24 ee ff ff       	call   10b0 <_gfortran_caf_sync_all@plt>
    228c:	48 8b 0d 75 2e 00 00 	mov    0x2e75(%rip),%rcx        # 5108 <positions.1+0x48>
    2293:	48 8b 15 76 2e 00 00 	mov    0x2e76(%rip),%rdx        # 5110 <positions.1+0x50>
    229a:	5f                   	pop    %rdi
    229b:	4c 8b 0d 1e 2e 00 00 	mov    0x2e1e(%rip),%r9        # 50c0 <positions.1>
    22a2:	48 8b 35 1f 2e 00 00 	mov    0x2e1f(%rip),%rsi        # 50c8 <positions.1+0x8>
    22a9:	48 8b 05 40 2e 00 00 	mov    0x2e40(%rip),%rax        # 50f0 <positions.1+0x30>
    22b0:	4c 8b 15 41 2e 00 00 	mov    0x2e41(%rip),%r10        # 50f8 <positions.1+0x38>
    22b7:	41 58                	pop    %r8
    22b9:	48 39 d1             	cmp    %rdx,%rcx
    22bc:	0f 8f 29 01 00 00    	jg     23eb <MAIN__+0x70b>
    22c2:	4c 8b 35 37 2e 00 00 	mov    0x2e37(%rip),%r14        # 5100 <positions.1+0x40>
    22c9:	4c 39 d0             	cmp    %r10,%rax
    22cc:	0f 8f 19 01 00 00    	jg     23eb <MAIN__+0x70b>
    22d2:	4c 89 f7             	mov    %r14,%rdi
    22d5:	c5 fb 10 0d 03 0f 00 	vmovsd 0xf03(%rip),%xmm1        # 31e0 <options.97.4+0x50>
    22dc:	00 
    22dd:	c5 fd 28 05 fb 0e 00 	vmovapd 0xefb(%rip),%ymm0        # 31e0 <options.97.4+0x50>
    22e4:	00 
    22e5:	4d 89 d7             	mov    %r10,%r15
    22e8:	48 0f af f9          	imul   %rcx,%rdi
    22ec:	48 ff c2             	inc    %rdx
    22ef:	49 29 c7             	sub    %rax,%r15
    22f2:	4d 8d 1c c1          	lea    (%r9,%rax,8),%r11
    22f6:	48 29 ca             	sub    %rcx,%rdx
    22f9:	49 8d 4f 01          	lea    0x1(%r15),%rcx
    22fd:	4c 89 9d 90 fd ff ff 	mov    %r11,-0x270(%rbp)
    2304:	4c 89 ad 80 fd ff ff 	mov    %r13,-0x280(%rbp)
    230b:	48 89 95 88 fd ff ff 	mov    %rdx,-0x278(%rbp)
    2312:	48 89 8d a8 fd ff ff 	mov    %rcx,-0x258(%rbp)
    2319:	48 01 f7             	add    %rsi,%rdi
    231c:	48 89 ce             	mov    %rcx,%rsi
    231f:	48 83 e1 fc          	and    $0xfffffffffffffffc,%rcx
    2323:	4c 8b ad 90 fd ff ff 	mov    -0x270(%rbp),%r13
    232a:	48 89 9d 90 fd ff ff 	mov    %rbx,-0x270(%rbp)
    2331:	48 8b 9d 88 fd ff ff 	mov    -0x278(%rbp),%rbx
    2338:	48 c1 ee 02          	shr    $0x2,%rsi
    233c:	48 8d 14 08          	lea    (%rax,%rcx,1),%rdx
    2340:	49 83 ff 02          	cmp    $0x2,%r15
    2344:	48 89 8d a0 fd ff ff 	mov    %rcx,-0x260(%rbp)
    234b:	48 0f 46 d0          	cmovbe %rax,%rdx
    234f:	45 31 c0             	xor    %r8d,%r8d
    2352:	48 89 95 98 fd ff ff 	mov    %rdx,-0x268(%rbp)
    2359:	4c 8d 5a 01          	lea    0x1(%rdx),%r11
    235d:	4c 8d 62 02          	lea    0x2(%rdx),%r12
    2361:	66 66 2e 0f 1f 84 00 	data16 cs nopw 0x0(%rax,%rax,1)
    2368:	00 00 00 00 
    236c:	0f 1f 40 00          	nopl   0x0(%rax)
    2370:	49 83 ff 02          	cmp    $0x2,%r15
    2374:	76 2e                	jbe    23a4 <MAIN__+0x6c4>
    2376:	49 8d 4c fd 00       	lea    0x0(%r13,%rdi,8),%rcx
    237b:	31 c0                	xor    %eax,%eax
    237d:	0f 1f 00             	nopl   (%rax)
    2380:	48 89 c2             	mov    %rax,%rdx
    2383:	48 ff c0             	inc    %rax
    2386:	48 c1 e2 05          	shl    $0x5,%rdx
    238a:	c5 fd 11 04 11       	vmovupd %ymm0,(%rcx,%rdx,1)
    238f:	48 39 c6             	cmp    %rax,%rsi
    2392:	75 ec                	jne    2380 <MAIN__+0x6a0>
    2394:	48 8b 8d a0 fd ff ff 	mov    -0x260(%rbp),%rcx
    239b:	48 39 8d a8 fd ff ff 	cmp    %rcx,-0x258(%rbp)
    23a2:	74 2e                	je     23d2 <MAIN__+0x6f2>
    23a4:	48 8b 85 98 fd ff ff 	mov    -0x268(%rbp),%rax
    23ab:	48 01 f8             	add    %rdi,%rax
    23ae:	c4 c1 7b 11 0c c1    	vmovsd %xmm1,(%r9,%rax,8)
    23b4:	4d 39 da             	cmp    %r11,%r10
    23b7:	7c 19                	jl     23d2 <MAIN__+0x6f2>
    23b9:	4a 8d 04 1f          	lea    (%rdi,%r11,1),%rax
    23bd:	c4 c1 7b 11 0c c1    	vmovsd %xmm1,(%r9,%rax,8)
    23c3:	4d 39 e2             	cmp    %r12,%r10
    23c6:	7c 0a                	jl     23d2 <MAIN__+0x6f2>
    23c8:	4a 8d 04 27          	lea    (%rdi,%r12,1),%rax
    23cc:	c4 c1 7b 11 0c c1    	vmovsd %xmm1,(%r9,%rax,8)
    23d2:	49 ff c0             	inc    %r8
    23d5:	4c 01 f7             	add    %r14,%rdi
    23d8:	4c 39 c3             	cmp    %r8,%rbx
    23db:	75 93                	jne    2370 <MAIN__+0x690>
    23dd:	4c 8b ad 80 fd ff ff 	mov    -0x280(%rbp),%r13
    23e4:	48 8b 9d 90 fd ff ff 	mov    -0x270(%rbp),%rbx
    23eb:	48 8b 15 7e 2d 00 00 	mov    0x2d7e(%rip),%rdx        # 5170 <masses.2+0x30>
    23f2:	48 8b 3d 7f 2d 00 00 	mov    0x2d7f(%rip),%rdi        # 5178 <masses.2+0x38>
    23f9:	4c 8b 05 40 2d 00 00 	mov    0x2d40(%rip),%r8        # 5140 <masses.2>
    2400:	4c 8b 0d 41 2d 00 00 	mov    0x2d41(%rip),%r9        # 5148 <masses.2+0x8>
    2407:	48 39 fa             	cmp    %rdi,%rdx
    240a:	0f 8f 89 00 00 00    	jg     2499 <MAIN__+0x7b9>
    2410:	48 89 f8             	mov    %rdi,%rax
    2413:	48 29 d0             	sub    %rdx,%rax
    2416:	4c 8d 58 01          	lea    0x1(%rax),%r11
    241a:	48 83 f8 02          	cmp    $0x2,%rax
    241e:	76 43                	jbe    2463 <MAIN__+0x783>
    2420:	c5 fd 28 05 b8 0d 00 	vmovapd 0xdb8(%rip),%ymm0        # 31e0 <options.97.4+0x50>
    2427:	00 
    2428:	49 8d 04 11          	lea    (%r9,%rdx,1),%rax
    242c:	4d 89 da             	mov    %r11,%r10
    242f:	49 8d 34 c0          	lea    (%r8,%rax,8),%rsi
    2433:	49 c1 ea 02          	shr    $0x2,%r10
    2437:	31 c0                	xor    %eax,%eax
    2439:	0f 1f 80 00 00 00 00 	nopl   0x0(%rax)
    2440:	48 89 c1             	mov    %rax,%rcx
    2443:	48 ff c0             	inc    %rax
    2446:	48 c1 e1 05          	shl    $0x5,%rcx
    244a:	c5 fd 11 04 0e       	vmovupd %ymm0,(%rsi,%rcx,1)
    244f:	49 39 c2             	cmp    %rax,%r10
    2452:	75 ec                	jne    2440 <MAIN__+0x760>
    2454:	4c 89 d8             	mov    %r11,%rax
    2457:	48 83 e0 fc          	and    $0xfffffffffffffffc,%rax
    245b:	48 01 c2             	add    %rax,%rdx
    245e:	4c 39 d8             	cmp    %r11,%rax
    2461:	74 36                	je     2499 <MAIN__+0x7b9>
    2463:	c5 fb 10 05 75 0d 00 	vmovsd 0xd75(%rip),%xmm0        # 31e0 <options.97.4+0x50>
    246a:	00 
    246b:	4a 8d 04 0a          	lea    (%rdx,%r9,1),%rax
    246f:	c4 c1 7b 11 04 c0    	vmovsd %xmm0,(%r8,%rax,8)
    2475:	48 8d 42 01          	lea    0x1(%rdx),%rax
    2479:	48 39 f8             	cmp    %rdi,%rax
    247c:	7f 1b                	jg     2499 <MAIN__+0x7b9>
    247e:	4c 01 c8             	add    %r9,%rax
    2481:	48 83 c2 02          	add    $0x2,%rdx
    2485:	c4 c1 7b 11 04 c0    	vmovsd %xmm0,(%r8,%rax,8)
    248b:	48 39 fa             	cmp    %rdi,%rdx
    248e:	7f 09                	jg     2499 <MAIN__+0x7b9>
    2490:	4c 01 ca             	add    %r9,%rdx
    2493:	c4 c1 7b 11 04 d0    	vmovsd %xmm0,(%r8,%rdx,8)
    2499:	48 8b 35 e8 2b 00 00 	mov    0x2be8(%rip),%rsi        # 5088 <velocities.0+0x48>
    24a0:	4c 8b 25 e9 2b 00 00 	mov    0x2be9(%rip),%r12        # 5090 <velocities.0+0x50>
    24a7:	48 8b 3d 92 2b 00 00 	mov    0x2b92(%rip),%rdi        # 5040 <velocities.0>
    24ae:	4c 8b 05 93 2b 00 00 	mov    0x2b93(%rip),%r8        # 5048 <velocities.0+0x8>
    24b5:	48 8b 0d b4 2b 00 00 	mov    0x2bb4(%rip),%rcx        # 5070 <velocities.0+0x30>
    24bc:	48 8b 15 b5 2b 00 00 	mov    0x2bb5(%rip),%rdx        # 5078 <velocities.0+0x38>
    24c3:	4c 39 e6             	cmp    %r12,%rsi
    24c6:	0f 8f 6f 01 00 00    	jg     263b <MAIN__+0x95b>
    24cc:	48 8b 05 ad 2b 00 00 	mov    0x2bad(%rip),%rax        # 5080 <velocities.0+0x40>
    24d3:	48 39 d1             	cmp    %rdx,%rcx
    24d6:	0f 8f 5f 01 00 00    	jg     263b <MAIN__+0x95b>
    24dc:	4c 8d 3c c5 00 00 00 	lea    0x0(,%rax,8),%r15
    24e3:	00 
    24e4:	48 0f af c6          	imul   %rsi,%rax
    24e8:	49 ff c4             	inc    %r12
    24eb:	48 89 9d a8 fd ff ff 	mov    %rbx,-0x258(%rbp)
    24f2:	49 29 f4             	sub    %rsi,%r12
    24f5:	4c 89 e6             	mov    %r12,%rsi
    24f8:	45 31 e4             	xor    %r12d,%r12d
    24fb:	4c 01 c0             	add    %r8,%rax
    24fe:	4c 89 e3             	mov    %r12,%rbx
    2501:	49 89 f4             	mov    %rsi,%r12
    2504:	48 01 c8             	add    %rcx,%rax
    2507:	48 8d 3c c7          	lea    (%rdi,%rax,8),%rdi
    250b:	48 89 d0             	mov    %rdx,%rax
    250e:	48 29 c8             	sub    %rcx,%rax
    2511:	4c 8d 34 c5 08 00 00 	lea    0x8(,%rax,8),%r14
    2518:	00 
    2519:	c5 f8 77             	vzeroupper 
    251c:	0f 1f 40 00          	nopl   0x0(%rax)
    2520:	4c 89 f2             	mov    %r14,%rdx
    2523:	31 f6                	xor    %esi,%esi
    2525:	48 ff c3             	inc    %rbx
    2528:	e8 03 eb ff ff       	call   1030 <memset@plt>
    252d:	48 89 c7             	mov    %rax,%rdi
    2530:	4c 01 ff             	add    %r15,%rdi
    2533:	49 39 dc             	cmp    %rbx,%r12
    2536:	75 e8                	jne    2520 <MAIN__+0x840>
    2538:	48 8b 9d a8 fd ff ff 	mov    -0x258(%rbp),%rbx
    253f:	48 8b 05 ca 0c 00 00 	mov    0xcca(%rip),%rax        # 3210 <options.97.4+0x80>
    2546:	48 89 85 c0 fd ff ff 	mov    %rax,-0x240(%rbp)
    254d:	31 d2                	xor    %edx,%edx
    254f:	31 f6                	xor    %esi,%esi
    2551:	31 ff                	xor    %edi,%edi
    2553:	e8 58 eb ff ff       	call   10b0 <_gfortran_caf_sync_all@plt>
    2558:	44 8b a5 b8 fd ff ff 	mov    -0x248(%rbp),%r12d
    255f:	45 85 e4             	test   %r12d,%r12d
    2562:	7e 6b                	jle    25cf <MAIN__+0x8ef>
    2564:	41 be 01 00 00 00    	mov    $0x1,%r14d
    256a:	66 0f 1f 44 00 00    	nopw   0x0(%rax,%rax,1)
    2570:	48 8d 05 29 2c 00 00 	lea    0x2c29(%rip),%rax        # 51a0 <accel.3>
    2577:	6a 00                	push   $0x0
    2579:	ff 35 91 2c 00 00    	push   0x2c91(%rip)        # 5210 <accel.3+0x70>
    257f:	6a 00                	push   $0x0
    2581:	ff 35 11 2c 00 00    	push   0x2c11(%rip)        # 5198 <masses.2+0x58>
    2587:	6a 00                	push   $0x0
    2589:	4d 89 e9             	mov    %r13,%r9
    258c:	49 89 d8             	mov    %rbx,%r8
    258f:	ff 35 1b 2b 00 00    	push   0x2b1b(%rip)        # 50b0 <velocities.0+0x70>
    2595:	6a 00                	push   $0x0
    2597:	41 ff c6             	inc    %r14d
    259a:	ff 35 90 2b 00 00    	push   0x2b90(%rip)        # 5130 <positions.1+0x70>
    25a0:	48 8b 08             	mov    (%rax),%rcx
    25a3:	48 8d 05 96 2b 00 00 	lea    0x2b96(%rip),%rax        # 5140 <masses.2>
    25aa:	48 8b 10             	mov    (%rax),%rdx
    25ad:	48 8d 05 8c 2a 00 00 	lea    0x2a8c(%rip),%rax        # 5040 <velocities.0>
    25b4:	48 8b 30             	mov    (%rax),%rsi
    25b7:	48 8d 05 02 2b 00 00 	lea    0x2b02(%rip),%rax        # 50c0 <positions.1>
    25be:	48 8b 38             	mov    (%rax),%rdi
    25c1:	e8 da f3 ff ff       	call   19a0 <__nbodymodule_MOD_advanceit>
    25c6:	48 83 c4 40          	add    $0x40,%rsp
    25ca:	45 39 f4             	cmp    %r14d,%r12d
    25cd:	7d a1                	jge    2570 <MAIN__+0x890>
    25cf:	48 8d 65 d0          	lea    -0x30(%rbp),%rsp
    25d3:	5b                   	pop    %rbx
    25d4:	41 5a                	pop    %r10
    25d6:	41 5c                	pop    %r12
    25d8:	41 5d                	pop    %r13
    25da:	41 5e                	pop    %r14
    25dc:	41 5f                	pop    %r15
    25de:	5d                   	pop    %rbp
    25df:	c3                   	ret    
    25e0:	44 89 f8             	mov    %r15d,%eax
    25e3:	44 31 e0             	xor    %r12d,%eax
    25e6:	79 03                	jns    25eb <MAIN__+0x90b>
    25e8:	44 01 e2             	add    %r12d,%edx
    25eb:	85 d2                	test   %edx,%edx
    25ed:	0f 84 f6 f8 ff ff    	je     1ee9 <MAIN__+0x209>
    25f3:	48 8b 05 0e 0c 00 00 	mov    0xc0e(%rip),%rax        # 3208 <options.97.4+0x78>
    25fa:	48 89 df             	mov    %rbx,%rdi
    25fd:	4c 89 b5 c8 fd ff ff 	mov    %r14,-0x238(%rbp)
    2604:	c7 85 d0 fd ff ff 64 	movl   $0x64,-0x230(%rbp)
    260b:	00 00 00 
    260e:	48 89 85 c0 fd ff ff 	mov    %rax,-0x240(%rbp)
    2615:	e8 46 eb ff ff       	call   1160 <_gfortran_st_write@plt>
    261a:	48 89 df             	mov    %rbx,%rdi
    261d:	ba 1c 00 00 00       	mov    $0x1c,%edx
    2622:	48 8d 35 05 0a 00 00 	lea    0xa05(%rip),%rsi        # 302e <_IO_stdin_used+0x2e>
    2629:	e8 62 ea ff ff       	call   1090 <_gfortran_transfer_character_write@plt>
    262e:	48 89 df             	mov    %rbx,%rdi
    2631:	e8 aa ea ff ff       	call   10e0 <_gfortran_st_write_done@plt>
    2636:	e9 ae f8 ff ff       	jmp    1ee9 <MAIN__+0x209>
    263b:	c5 f8 77             	vzeroupper 
    263e:	e9 fc fe ff ff       	jmp    253f <MAIN__+0x85f>
    2643:	ba 18 00 00 00       	mov    $0x18,%edx
    2648:	48 8d 35 b5 09 00 00 	lea    0x9b5(%rip),%rsi        # 3004 <_IO_stdin_used+0x4>
    264f:	48 8d 3d 1a 0a 00 00 	lea    0xa1a(%rip),%rdi        # 3070 <_IO_stdin_used+0x70>
    2656:	31 c0                	xor    %eax,%eax
    2658:	e8 43 ea ff ff       	call   10a0 <_gfortran_os_error_at@plt>
    265d:	48 8d 15 02 0a 00 00 	lea    0xa02(%rip),%rdx        # 3066 <_IO_stdin_used+0x66>
    2664:	48 8d 35 35 0a 00 00 	lea    0xa35(%rip),%rsi        # 30a0 <_IO_stdin_used+0xa0>
    266b:	48 8d 3d de 0a 00 00 	lea    0xade(%rip),%rdi        # 3150 <_IO_stdin_used+0x150>
    2672:	31 c0                	xor    %eax,%eax
    2674:	e8 c7 e9 ff ff       	call   1040 <_gfortran_runtime_error_at@plt>
    2679:	48 8d 15 e0 09 00 00 	lea    0x9e0(%rip),%rdx        # 3060 <_IO_stdin_used+0x60>
    2680:	48 8d 35 19 0a 00 00 	lea    0xa19(%rip),%rsi        # 30a0 <_IO_stdin_used+0xa0>
    2687:	48 8d 3d 9a 0a 00 00 	lea    0xa9a(%rip),%rdi        # 3128 <_IO_stdin_used+0x128>
    268e:	31 c0                	xor    %eax,%eax
    2690:	e8 ab e9 ff ff       	call   1040 <_gfortran_runtime_error_at@plt>
    2695:	48 8d 15 b9 09 00 00 	lea    0x9b9(%rip),%rdx        # 3055 <_IO_stdin_used+0x55>
    269c:	48 8d 35 fd 09 00 00 	lea    0x9fd(%rip),%rsi        # 30a0 <_IO_stdin_used+0xa0>
    26a3:	48 8d 3d 56 0a 00 00 	lea    0xa56(%rip),%rdi        # 3100 <_IO_stdin_used+0x100>
    26aa:	31 c0                	xor    %eax,%eax
    26ac:	e8 8f e9 ff ff       	call   1040 <_gfortran_runtime_error_at@plt>
    26b1:	48 8d 15 93 09 00 00 	lea    0x993(%rip),%rdx        # 304b <_IO_stdin_used+0x4b>
    26b8:	48 8d 35 e1 09 00 00 	lea    0x9e1(%rip),%rsi        # 30a0 <_IO_stdin_used+0xa0>
    26bf:	48 8d 3d 12 0a 00 00 	lea    0xa12(%rip),%rdi        # 30d8 <_IO_stdin_used+0xd8>
    26c6:	31 c0                	xor    %eax,%eax
    26c8:	e8 73 e9 ff ff       	call   1040 <_gfortran_runtime_error_at@plt>

Disassembly of section .fini:

00000000000026d0 <_fini>:
    26d0:	f3 0f 1e fa          	endbr64 
    26d4:	48 83 ec 08          	sub    $0x8,%rsp
    26d8:	48 83 c4 08          	add    $0x8,%rsp
    26dc:	c3                   	ret    
