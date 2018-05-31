import ollyapi as oa
from os import path
t = oa.pluginvalue_to_t_table(oa.Plugingetvalue(oa.VAL_MODULES))
for i in xrange(t.data.n):
  m = oa.void_to_t_module(oa.Getsortedbyselection(t.data, i))
  print '%s : <%08X, %08X>' % (path.basename(m.path), m.base, m.size)
  mi = {
     'path': m.path,
     'name': path.splitext(path.basename(m.path))[0],
     'base': m.base,
     'size': m.size
  }
  externals = list()
  for off in xrange(m.codesize):
    name = bytearray(oa.TEXTLEN)
    if oa.Findname(m.codebase + off, oa.NM_EXPORT, name):
      externals.append({'ea': m.codebase + off, 'name': str(name.replace('\x00', ''))})
  mi['apis'] = externals
  print mi
del t

import ollyapi as oa
symb = bytearray(2048)
comment = bytearray(oa.TEXTLEN)

n = oa.Decodeaddress(0x34114c, 0, oa.ADC_SYMBOL | oa.ADC_ENTRY, symb, 2048, comment)
print 'n: %s, symb: %s, comment: %s' % (n, symb.replace('\x00', ''), comment.replace('\x00', ''))


import ollyutils
print ollyutils.analyze_external_refs(0x34114c, 0x341150, 1)

th = oa.Findthread(oa.Getcputhreadid())
import inspect
r = oa.ulongArray.frompointer(th.reg.r)
for i, v in enumerate(r):
  if i > 7: break
  print "%r: %08x" % (i, v)
print 'eip: %08X' % th.reg.ip


# ---------------------
import ollyutils
size, mem = ollyutils.safe_read_chunked_memory_region_as_one(0x6ef40000, 0x190000);
print type(mem)
with open('d:\\pe_test.bin', 'wb') as f:
  f.write(mem)
t = ctypes.ARRAY(ctypes.c_ubyte, size)
_data = t(*[n for n in mem])
print _data
pdata = ctypes.POINTER(t).from_address(ctypes.addressof(_data))
for k, v in inspect.getmembers(pdata.contents):
  print "%r: %r" % (k, v)
def cast_rva(rva, type_):
  #va = pdata + rva
  return ctypes.cast(ctypes.POINTER(t).from_address(ctypes.addressof(_data) + rva), type_)

dos_header = cast_rva(0, PIMAGE_DOS_HEADER)[0]
print 'e_magic: %x, e_cblp: %08X' % (dos_header.e_magic, dos_header.e_cblp)
for i in xrange(6):
  print '%02x\n' % _data[i]
del mem

# ---------------------

import ollyapi as oa
import ollyutils
name = bytearray(oa.TEXTLEN)
dis = oa.t_disasm()
mem = ollyutils.safe_read_chunked_memory_region_as_one(ea_from, ea_to - ea_from)
n = oa.Disasm(cmd, len(cmd), ea, None, dis, oa.DISASM_CODE, 0)

# ---------------------
# CRIDEX GPA enum creation
ref = idaapi.get_first_cref_to(0x004049C6) # addr of fmfu_GPA func

def getProcEnumName(idx):
	e = idx * 4 + 0x410020 # addr of beginnig of procs array
	l = idaapi.get_long(e)
	oaep = GetEnum('OLD_API_EXTERN_CONSTS')
	member = idaapi.get_enum_member(oaep, l, 0, 0)
	if member == idaapi.BADADDR:
		return None
	n = idaapi.get_enum_member_name(member)
	return n

ids = set()
while ref != idaapi.BADADDR:
	items = [x for x in FuncItems(ref)]
	if ref in items:
		num = items.index(ref) - 1
		ea = items[num]
		if GetMnem(ea) == 'push':
			v = GetOpnd(ea, 0)
			if 'h' in v:
				v = v.replace('h', '')
			try:
				v = int(v, 16)
				gpa = GetEnum('GPA')
				existing = idaapi.get_enum_member(gpa, v, 0, 0)
				if existing != idaapi.BADADDR:
					
					print 'making GPA enum member at ea: %08X %d' % (ea, idaapi.op_enum(ea, 0, gpa, 0))
				else:
					n = getProcEnumName(v)
					if not n:
						print 'ea %08X skipped' % ea
					else:
						n = n.replace('OAEC_', '')
						module, name = n.split('_')
						gpa_name = 'GPA_%s.%s' % (module, name)
				
						en = GetEnum('GPA')
						idaapi.begin_type_updating(idaapi.UTP_ENUM)
						idaapi.add_enum_member(gpa, gpa_name, v)
						idaapi.end_type_updating(idaapi.UTP_ENUM)
						idaapi.op_enum(ea, 0, gpa, 0)
						print 'processed ea: %08X as %s' % (ea, gpa_name)
			except ValueError:
				print 'bad operand at ea: %08X' % ea
	ref = idaapi.get_next_cref_to(0x004049C6, ref)
print ids

# --------------------------- mofa strings
fmfu_loadFromEncrypted_ea = 0x403D32
g_encStrings = 0x40DD18

key = [idaapi.get_byte(x) for x in range(g_encStrings + 0x8, g_encStrings + 0x10)]
strings = []
ea = g_encStrings + 0x10

decr = []
for e in xrange(0x10000):
	decrypted_part = []
	for i in xrange(8):
		decrypted_part.append(idaapi.get_byte(ea + i) ^ key[i])
	decr += decrypted_part
	check = decr[-9:]

	ea += 8
	
with open('d:\\cridex_%08X.bin' % g_encStrings, 'wb') as f:
	f.write(bytearray(decr))
#-------------------------
target_func = 0x00408612


def get_int_opnd(ea, n):
	op = GetOpnd(ea, n)
	if not op:
		return None
	if 'h' == op[-1]:
		op = op[:-1]
	return int(op, 16)

def get_args(ea):
	arg_ecx = None

	tmp_ea = ea
	while True:
		c = DecodePreviousInstruction(tmp_ea)
		if not c or  not c.is_canon_insn():
			return None

		mnem = c.get_canon_mnem()

		is_mov_ecx_xxx = mnem == 'mov' and c.Op1.type == idaapi.o_reg and GetOpnd(c.ea, 0) == 'ecx'
		is_pop_ecx = mnem == 'pop' and  c.Op1.type == idaapi.o_reg and GetOpnd(c.ea, 0) == 'ecx'
		is_xor_ecx_ecx = mnem == 'xor' and c.Op1.type == idaapi.o_reg and c.Op2.type == idaapi.o_reg and GetOpnd(c.ea, 0) == GetOpnd(c.ea, 1) == 'ecx'

		if is_xor_ecx_ecx:
			return 0
		if is_mov_ecx_xxx or is_pop_ecx:
			imm = None
			regname =  GetOpnd(c.ea, 1) if is_mov_ecx_xxx else GetOpnd(c.ea, 0)
			if (is_mov_ecx_xxx and c.Op2.type == idaapi.o_reg) or (is_pop_ecx and c.Op1.type == idaapi.o_reg):
				ea2 = c.ea
				while True:
					c2 = DecodePreviousInstruction(ea2)
					if not c2:
						break	
					mnem = c2.get_canon_mnem()
					if mnem == 'mov' and GetOpnd(c2.ea, 0) == regname:
						return c2.Op2.value
					elif is_pop_ecx and mnem == 'push':
						if c2.Op1.type == idaapi.o_imm:
							return c2.Op1.value			

					ea2 = c2.ea
				if not imm:
					print 'decode args failed: 0x%08x' % ea
					return None
			else:
				return c.Op2.value

		if arg_ecx:
			return arg_ecx

		tmp_ea = c.ea

	return arg_ecx


def decrypt_string(index):
	table = 0x4222F2
	size = idaapi.get_word(table + index * 8)
	ptr = idaapi.get_long(0x4222F4 + index * 8)
	rv = ''
	for i in range(size):
		ch = idaapi.get_byte(ptr + i)
		ch ^= idaapi.get_byte(0x4222F0 + index * 8)
		ch ^= i
		rv += chr(ch & 0xFF)
	return rv

found = 0
added = 0
ref = idaapi.get_first_cref_to(target_func)
while ref != idaapi.BADADDR:
	arg_ecx = get_args(ref)
	print 'ref at %08x, ecx: %08X' % (ref, arg_ecx if not arg_ecx  is None else -1)
	found += 1
	if not arg_ecx is None:
		s = decrypt_string(arg_ecx)
		print 'decrypted: %s' % s

		idaapi.set_cmt(ref, '"%s"' %s, False)
	
	ref = idaapi.get_next_cref_to(target_func, ref)

with open('d:\\mofa.bin', 'wb') as f:
	for i in range(0x150):
		try:
			s = decrypt_string(i)
			f.write('%08X: %s\r\n' % (i, s))
		except:
			exit()
# ----------------------
ea = 0x9B3827
for i in range(0x100):
	e = ea + i * 4
	idaapi.doDwrd(e, 4)
	v = idaapi.get_long(e)
	v ^= 0x43198A3B
	v += 0xF09B0000
	v &= 0xFFFFFFFF
	s =  hex(int(v))
	idaapi.set_cmt(e, '', False)
	idaapi.patch_long(e, v)
	OpOff(e, 0, 0);
	print s
