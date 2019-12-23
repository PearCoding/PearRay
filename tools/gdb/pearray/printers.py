# -*- coding: utf-8 -*-

# How to enable it:
# * Create a ~/.gdbinit file, that contains the following:
#      python
#      import sys
#      sys.path.insert(0, '/path/to/pearray/printer/directory')
#      from pearray.printers import register_pearray_printers
#      register_pearray_printers (None)
#      end

import gdb
import re


class _SpectrumEntryIterator(object):
	"Internal Spectrum Entry Interator"

	def __init__ (self, entries):
		self.entries = entries
		self.currentEntry = 0

	def __iter__ (self):
		return self

	def next(self):
		return self.__next__()  # Python 2.x compatibility

	def __next__(self):
		entry = self.currentEntry
		if self.currentEntry >= self.entries:
			raise StopIteration

		self.currentEntry = self.currentEntry + 1
		return entry


class SpectrumPrinter:
	"Print PR::Spectrum"

	def __init__(self, val):
		"Extract all the necessary information"

		# mInternal is a shared_ptr!
		sharedData = val['mInternal']['_M_ptr'].dereference()
		self.data = sharedData['Data']
		self.entries = int(sharedData['End']) - int(sharedData['Start'])

	class _iterator(_SpectrumEntryIterator):
		def __init__ (self, entries, dataPtr):
			super(SpectrumPrinter._iterator, self).__init__(entries)
			self.dataPtr = dataPtr

		def __next__(self):
			entry = super(SpectrumPrinter._iterator, self).__next__()

			item = self.dataPtr.dereference()
			self.dataPtr = self.dataPtr + 1
			return ('[%d]' % (entry,), item)

	def children(self):
		return self._iterator(self.entries, self.data)

	def to_string(self):
		return "PR::Spectrum[%d] (data ptr: %s)" % (self.entries, self.data)


class VPrinter:
	"Print vfloat/vuint32/vint32"

	def __init__(self, val):
		"Extract all the necessary information"

		self.data = []
		self.data.append(val['d_'][0])
		self.data.append(val['d_'][1])
		self.data.append(val['d_'][2])
		self.data.append(val['d_'][3])

	def to_string(self):
		return "[%s]" % (",".join(str(f) for f in self.data))


def build_pearray_dictionary ():
	pretty_printers_dict[re.compile('^PR::Spectrum$')] = lambda val: SpectrumPrinter(val)
	pretty_printers_dict[re.compile('PR::vfloat')] = lambda val: VPrinter(val)
	pretty_printers_dict[re.compile('^simdpp::\\w+::float32<4, void>$')] = lambda val: VPrinter(val)
	pretty_printers_dict[re.compile('^PR::vint32$')] = lambda val: VPrinter(val)
	pretty_printers_dict[re.compile('^simdpp::\\w+::int32<4, void>$')] = lambda val: VPrinter(val)
	pretty_printers_dict[re.compile('^PR::vuint32$')] = lambda val: VPrinter(val)
	pretty_printers_dict[re.compile('^simdpp::\\w+::uint32<4, void>$')] = lambda val: VPrinter(val)


def register_pearray_printers(obj):
	"Register pearray pretty-printers with objfile Obj"

	if obj is None:
		obj = gdb
	obj.pretty_printers.append(lookup_function)


def lookup_function(val):
	"Look-up and return a pretty-printer that can print va."

	type = val.type

	if type.code == gdb.TYPE_CODE_REF:
		type = type.target()

	type = type.unqualified().strip_typedefs()

	typename = type.tag
	if typename is None:
		return None

	#print(typename)
	for function in pretty_printers_dict:
		if function.search(typename):
			return pretty_printers_dict[function](val)

	return None


pretty_printers_dict = {}
build_pearray_dictionary ()
