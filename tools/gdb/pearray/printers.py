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


class SIMDPrinter:
	"Print vfloat/vuint32/vint32"

	def __init__(self, val, inner_type):
		"Extract all the necessary information"

		ptr_type = gdb.lookup_type("%s" % inner_type).pointer()
		ptr = val.address.reinterpret_cast(ptr_type)
		self.data = []
		self.data.append(ptr[0])
		self.data.append(ptr[1])
		self.data.append(ptr[2])
		self.data.append(ptr[3])

	def to_string(self):
		return "[%s]" % (",".join(str(f) for f in self.data))

	#def display_hint(self):
#		return 'array'

	def children(self):
		l = list((str(c), value) for c, value in enumerate(self.data))
		return iter(l)


def build_pearray_dictionary ():
	pretty_printers_dict[re.compile('^PR::Spectrum$')] = lambda val: SpectrumPrinter(val)
	pretty_printers_dict[re.compile('^PR::VectorBase<4, float')] = lambda val: SIMDPrinter(val, 'float')
	pretty_printers_dict[re.compile('^PR::VectorBase<4, int')] = lambda val: SIMDPrinter(val, 'int')
	pretty_printers_dict[re.compile('^PR::VectorBase<4, unsigned int')] = lambda val: SIMDPrinter(val, 'unsigned int')


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
