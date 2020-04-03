import base64
import string
import idaapi
import traceback
my_base64chars  = "vQ9xGcLbYjrSNA2syoifODkuhtC+FUp7EZXqldgRnPw08/Jm16MezK5BV4IaW3TH"
std_base64chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"

def decrypt_str(raw):
	for padding in xrange(4):
		enc = raw + '=' * padding
		try:
			s = enc.translate(string.maketrans(my_base64chars, std_base64chars))
			data = base64.b64decode(s)
			return data
		except: pass

MAX_STR = 0x10000
def get_str_at(p):
	rv = ''
	idx = 0
	while len(rv) < MAX_STR:
		c = idaapi.get_byte(p)
		p += 1
		if not c:
			break
		rv += chr(c)
		
	return rv



ea = 0x41536C # 0x04157C8
end_ea = 0x416A6F # 0x415CC8
idx = 0
while ea < end_ea:
	s = get_str_at(ea)
	if not s: break
	idx += 1
	if not s:
		print '- cannot get str at %08x' % ea
		break
	
	try:
		decr = decrypt_str(s)
		print '#%03x %08x: %s' % (idx, ea, decr)
		idaapi.set_cmt(ea, decr, True)
	except:
		print 'cannot decode string at %08x "%s": %s' % (ea, s, traceback.format_exc())
	ea += len(s) + 1
