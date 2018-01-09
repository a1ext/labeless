#pragma once

#include "types.h"

#ifdef __NT__
#pragma warning(push)
#pragma warning(disable:4309 4244 4267)           // disable "truncation of constant value" warning from IDA SDK, conversion from 'ssize_t' to 'int', possible loss of data
#endif // __NT__
#include <auto.hpp>
#include <entry.hpp>
#include <loader.hpp>
#include <struct.hpp>
#include <name.hpp>
#ifdef __NT__
#pragma warning(pop)
#endif // __NT__

namespace compat {
	/**
	** compatibility layer for different IDA versions
	**/
#if (IDA_SDK_VERSION < 700)
	typedef ::area_t IDARange;
	typedef ::TForm TWidget;
	typedef void printop_t;

#define START_RANGE_EA(X) (X)->startEA
#define END_RANGE_EA(X) (X)->endEA
#define INSN_T_OPNDS(X) (X)->Operands
#define NETNODE_ALT_FIRST(X) (X)->alt1st()
#define NETNODE_ALT_FIRST_TAG(X, T) (X)->alt1st((T))
#define NETNODE_ALT_NEXT(X) (X)->altnxt(*(X))
#define NETNODE_ALT_NEXT_TAG(X) (X)->altnxt(*(X), (T))
#define NETNODE_SUP_FIRST(X) (X)->sup1st()
#define NETNODE_SUP_FIRST_TAG(X, T) (X)->sup1st((T))
#define NETNODE_SUP_NEXT(X, IDX) (X)->supnxt(*(X))
#define NETNODE_SUP_NEXT_TAG(X, IDX, T) (X)->supnxt(*(X), (T))

	inline flags_t get_flags(ea_t ea) { return ::get_flags_novalue(ea); }
	inline int decode_insn(::insn_t* outInsn, ea_t ea) {
		const int rv = ::decode_insn(ea);
		if (rv < 0)
			return rv;
		if (outInsn)
			*outInsn = ::cmd;
		return rv;
	}
	inline bool is_stkvar0(flags_t F) { return ::isStkvar0(F); }
	inline bool is_stkvar1(flags_t F) { return ::isStkvar1(F); }
	inline bool is_struct(flags_t F) { return ::isStruct(F); }
	inline bool is_stroff(flags_t F, int n) { return ::isStroff(F, n); }
	inline bool is_stroff0(flags_t F) { return ::isStroff0(F); }
	inline bool is_stroff1(flags_t F) { return ::isStroff1(F); }
	inline bool is_code(flags_t F) { return ::isCode(F); }
	inline bool is_data(flags_t F) { return ::isData(F); }
	inline ea_t to_ea(sel_t reg_cs, ea_t reg_ip) { return ::toEA(reg_cs, reg_ip); }
	inline bool is_enabled(ea_t ea) { return ::isEnabled(ea); }
	inline const char* get_idb_path() { return ::database_idb; }
	inline bool get_member_name(qstring *out, tid_t mid) { return ::get_member_name2(out, mid) > 0; }
	inline int get_struct_operand(adiff_t *disp, adiff_t *delta, tid_t *path, ea_t ea, int n) { return ::get_struct_operand(ea, n, path, disp, delta); }
	inline flags_t append_struct_fields(qstring *out, adiff_t *disp, int n, const tid_t *path, int plen, flags_t flags, adiff_t delta, bool appzero) {
		return ::append_struct_fields2(out, n, path, plen, flags, disp, delta, appzero);
	}
	inline flags_t byte_flag(void) { return ::byteflag(); }
	inline int get_stroff_path(tid_t *path, adiff_t *delta, ea_t ea, int n) {
		return ::get_stroff_path(ea, n, path, delta);
	}
	inline void append_disp(qstring *buf, adiff_t disp, bool tag = true) {
		if (!buf)
			return;

		::qstring tmp;

		tmp.resize(MAXSTR + 1);
		char* p = &tmp[0];
		auto len = ::print_disp(p, p + MAXSTR, disp);
		if (len > 0)
		{
			tmp.resize(len);
			*buf += tmp;
		}
	}
	inline ssize_t get_func_cmt(qstring *buf, const func_t *pfn, bool repeatable) {
		if (!buf || !pfn)
			return 0;

		const char* funcCmt = ::get_func_cmt(const_cast<func_t*>(pfn), repeatable);
		if (!funcCmt)
			return 0;

		*buf = funcCmt;

		return static_cast<ssize_t>(buf->length());
	}
	inline ssize_t get_cmt(qstring *buf, ea_t ea, bool rptble) {
		if (!buf)
			return 0;

		ssize_t len = ::get_cmt(ea, rptble, nullptr, 0);
		if (len <= 0)
			return len;
		buf->resize(len + 1);
		len = ::get_cmt(ea, rptble, &(*buf)[0], len);
		buf->resize(len);
		return static_cast<ssize_t>(buf->length());
	}
	inline bool auto_wait() { return ::autoWait(); }
	inline bool is_call_insn(const ::insn_t& insn) { return ::is_call_insn(insn.ea); }
	inline bool print_operand(qstring *out, ea_t ea, int n, int getn_flags = 0, printop_t *newtype = NULL) {
		if (!out)
			return false;

		out->resize(MAXSTR + 1, '\0');
		if (!ua_outop2(ea, &(*out)[0], MAXSTR, n, getn_flags))
		{
			out->clear();
			return false;
		}
		if (!tag_remove(&(*out)[0], &(*out)[0], MAXSTR))
		{
			out->clear();
			return false;
		}
		out->resize(::qstrlen(out->c_str()));
		return !out->empty();
	}
	using ::do_unknown;
	using ::do_unknown_range;
	inline void activate_widget(TWidget *widget, bool take_focus) { return ::switchto_tform(widget, take_focus); }
	inline TWidget *create_empty_widget(const char *title, int icon = -1) {
		HWND h = NULL;
		return ::create_tform(title, &h);
	}
	inline void display_widget(TWidget *widget, int options) {
		if (!widget)
			return;
		::open_tform(widget, options);
	}
	inline bool print_insn_mnem(qstring *out, ea_t ea) {
		if (out)
			return false;

		out->resize(MAXSTR + 1);
		if (!ua_mnem(ea, &(*out)[0], MAXSTR))
			return false;

		out->resize(::qstrlen(out->c_str()));
		return !out->empty();
	}
	inline bool force_name(ea_t ea, const char *name, int flags = 0) {
		return ::do_name_anyway(ea, name, 0);
	}
	inline uint32 get_dword(ea_t ea) { return ::get_long(ea); }
	inline ssize_t get_segm_name(qstring *buf, const segment_t *s, int flags = 0) {
		if (!buf || !s)
			return -1;

		buf->resize(MAXSTR + 1);

		ssize_t len = ::get_segm_name(s, &(*buf)[0], MAXSTR);
		if (len <= 0)
		{
			buf->clear();
			return len;
		}
		buf->resize(len);
		return static_cast<ssize_t>(buf->length());
	}
	inline ssize_t get_entry_name(qstring *buf, uval_t ord) {
		if (!buf)
			return -1;

		buf->resize(MAXSTR + 1);
		ssize_t len = ::get_entry_name(ord, &(*buf)[0], MAXSTR);
		if (len <= 0)
		{
			buf->clear();
			return len;
		}
		buf->resize(len);
		return static_cast<ssize_t>(buf->length());
	}
	inline bool is_dword(flags_t F) { return ::isDwrd(F); }
	inline bool is_qword(flags_t F) { return ::isQwrd(F); }
	inline bool create_dword(ea_t ea, asize_t length) { return ::doDwrd(ea, length); }
	inline bool create_qword(ea_t ea, asize_t length) { return ::doQwrd(ea, length); }

#define PROCESSOR_T_NEWFILE (::processor_t::newfile)
#define PROCESSOR_T_OLDFILE (::processor_t::oldfile)
#define PROCESSOR_T_INIT (::processor_t::init)
#define PROCESSOR_T_TERM (::processor_t::term)
#define PROCESSOR_T_RENAME (::processor_t::rename)
#define PROCESSOR_T_GEN_REGVAR_DEF (::processor_t::gen_regvar_def)
#define PROCESSOR_T_ADD_CREF (::processor_t::add_cref)
#ifdef __NT__
#	define PROCESSOR_T_ADD_DREF (::processor_t::add_dref)
#endif // __NT__

#define ASK_FORM(X, ...) ::AskUsingForm_c((X), __VA_ARGS__)
#define ASK_YN(X, ...) ::askyn_c((X), __VA_ARGS__)
#define HOOK_CB_T_RET_TYPE int;

#else // IDA_SDK_VERSION < 700
	typedef ::range_t IDARange;
	typedef ::TWidget TWidget;

#define START_RANGE_EA(X) (X)->start_ea
#define END_RANGE_EA(X) (X)->end_ea
#define INSN_T_OPNDS(X) (X)->ops
#define NETNODE_ALT_FIRST(X) (X)->altfirst()
#define NETNODE_ALT_FIRST_TAG(X, T) (X)->altfirst((T))
#define NETNODE_ALT_NEXT(X) (X)->altnext(*(X))
#define NETNODE_ALT_NEXT_TAG(X) (X)->altnext(*(X), (T))
#define NETNODE_SUP_FIRST(X) (X)->supfirst()
#define NETNODE_SUP_FIRST_TAG(X, T) (X)->supfirst((T))
#define NETNODE_SUP_NEXT(X, IDX) (X)->supnext((IDX))
#define NETNODE_SUP_NEXT_TAG(X, IDX, T) (X)->supnext((IDX), (T))

	using ::get_flags;
	using ::decode_insn;
	using ::is_stkvar0;
	using ::is_stkvar1;
	using ::is_struct;
	using ::is_stroff;
	using ::is_stroff0;
	using ::is_stroff1;
	using ::is_code;
	using ::is_data;
	using ::to_ea;
	inline bool get_member_name(qstring *out, tid_t mid) { return ::get_member_name(out, mid) > 0; }
	inline bool is_enabled(ea_t ea) { return ::is_mapped(ea); }
	inline const char* get_idb_path() { return ::get_path(::PATH_TYPE_IDB); }
	using ::get_struct_operand;
	using ::append_struct_fields;
	using ::byte_flag;
	using ::get_stroff_path;
	using ::append_disp;
	using ::get_func_cmt;
	using ::get_cmt;
	using ::auto_wait;
	using ::is_call_insn;
	using ::print_operand;
	inline bool do_unknown(ea_t ea, int flags) { return ::del_items(ea, flags); }
	inline void do_unknown_range(ea_t ea, size_t size, int flags) {
		::del_items(ea, flags, static_cast<::asize_t>(size));
	}
	using ::activate_widget;
	using ::create_empty_widget;
	using ::display_widget;
	using ::print_insn_mnem;
	using ::force_name;
	using ::get_dword;
	using ::get_segm_name;
	using ::get_entry_name;
	using ::is_dword;
	using ::is_qword;
	using ::create_dword;
	using ::create_qword;

#define PROCESSOR_T_NEWFILE (::processor_t::ev_newfile)
#define PROCESSOR_T_OLDFILE (::processor_t::ev_oldfile)
#define PROCESSOR_T_INIT (::processor_t::ev_init)
#define PROCESSOR_T_TERM (::processor_t::ev_term)
#define PROCESSOR_T_RENAME (::processor_t::ev_rename)
#define PROCESSOR_T_GEN_REGVAR_DEF (::processor_t::ev_gen_regvar_def)
#define PROCESSOR_T_ADD_CREF (::processor_t::ev_add_cref)
#ifdef __NT__
#	define PROCESSOR_T_ADD_DREF (::processor_t::ev_add_dref)
#endif // __NT__
#define ASK_FORM(X, ...) ::ask_form((X), __VA_ARGS__)
#define ASK_YN(DFLT, X, ...) ::ask_yn((DFLT), (X), __VA_ARGS__)
#define HOOK_CB_T_RET_TYPE ::ssize_t;

#endif // IDA_SDK_VERSION < 700
} // compat
