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

	def __init__(self, val):
		"Extract all the necessary information"

		vector_type = str(val.type.unqualified())

		inner_type = None
		if vector_type == 'PR::vfloat':
			inner_type = 'float'
		elif vector_type == 'PR::vint32':
			inner_type = 'int'
		elif vector_type == 'PR::vuint32':
			inner_type = 'unsigned int'
		else:
			match = re.match('^PR::VectorBase<4, (\\w+),', vector_type)
			if match is None:
				print("ERROR: Unknown vector type %s" % str(vector_type))
			else:
				inner_type = match[1]

		ptr_type = gdb.lookup_type(inner_type).pointer()
		ptr = val['d_'].address.reinterpret_cast(ptr_type)
		self.data = []
		self.data.append(ptr[0])
		self.data.append(ptr[1])
		self.data.append(ptr[2])
		self.data.append(ptr[3])

	def to_string(self):
		return ("[%s]" % (",".join(str(f) for f in self.data)))

	def display_hint(self):
		return 'array'

	def children(self):
		l = list((str(c), value) for c, value in enumerate(self.data))
		return iter(l)


# TODO: Add support for custom matrix sizes
class EigenSIMDPrinter:
	"Print Vector3fv, Vector3iv"

	def __init__(self, val):
		"Extract all the necessary information"

		dataptr = val['m_storage']['m_data']['array']
		self.data = [dataptr[0], dataptr[1], dataptr[2]]

	def to_string(self):
		return self.data

	def display_hint(self):
		return 'array'

	def children(self):
		l = list((str(c), value) for c, value in enumerate(self.data))
		return iter(l)


def build_pretty_printer ():
	pp = gdb.printing.RegexpCollectionPrettyPrinter("pearray")
	pp.add_printer('Spectrum', '^PR::Spectrum$', SpectrumPrinter)
	pp.add_printer('VectorBase', '^PR::VectorBase', SIMDPrinter)
	pp.add_printer('VectorV', 'Eigen::Matrix<PR::VectorBase', EigenSIMDPrinter)
	return pp


def register_pearray_printers(obj):
	gdb.printing.register_pretty_printer(
		gdb.current_objfile() if obj is None else obj,
		build_pretty_printer())
