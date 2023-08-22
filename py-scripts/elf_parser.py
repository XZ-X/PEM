from elftools.elf.elffile import ELFFile
from elftools.elf.relocation import RelocationSection
from elftools.elf.sections import SymbolTableSection
import sys

def get_symbol_type(dyn_symbol):
    if 'st_info' in dyn_symbol.entry and 'type' in dyn_symbol.entry['st_info']:
        type_str = dyn_symbol.entry['st_info']['type']
        # return type_str
        if 'FUNC' in type_str:
            return 'F'
        elif 'OBJEC' in type_str:
            return 'O'
    return 'F'


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Specify input file!")
        exit(-1)
    result = {}
    # bin_file = open(dbg_path, 'rb')
    bin_file = open(sys.argv[1], 'rb')
    out_file = open(sys.argv[1]+'.plt', 'w')
    elf_file = ELFFile(bin_file)
    reloc_section: RelocationSection = elf_file.get_section_by_name('.rela.plt')
    relocs = [i for i in reloc_section.iter_relocations()]
    symb_tbl_section: SymbolTableSection = elf_file.get_section_by_name(
        '.dynsym')
    for reloc in relocs:
      ofs = reloc['r_offset']
      sym_idx = reloc['r_info_sym']
      if sym_idx == 0:
          continue
      symbol = symb_tbl_section.get_symbol(sym_idx)      
      if symbol.name == "":
        result[ofs] = "None"
        print("0x%0x\t%s\t%s"%(ofs, get_symbol_type(symbol), "None"))
      else:
        result[ofs] = symbol.name
        print("0x%0x\t%s\t%s"%(ofs, get_symbol_type(symbol), symbol.name))
    reloc_dyn_section: RelocationSection = elf_file.get_section_by_name('.rela.dyn')
    relocs = [i for i in reloc_dyn_section.iter_relocations()]
    for reloc in relocs: 
      ofs = reloc['r_offset']
      sym_idx = reloc['r_info_sym']
      if sym_idx == 0:
          continue
      symbol = symb_tbl_section.get_symbol(sym_idx)
      if symbol.name == "":
        result[ofs] = "None"
        print("0x%0x\t%s\t%s"%(ofs, get_symbol_type(symbol), "None"))
      else:
        result[ofs] = symbol.name      
        print("0x%0x\t%s\t%s"%(ofs, get_symbol_type(symbol), symbol.name))
    for symb in symb_tbl_section.iter_symbols():
        if 'st_value' in symb.entry and symb.entry['st_value'] != 0:
            addr = symb.entry['st_value']
            if symb.name == "":
                result[addr] = "None"
                print("0x%0x\t%s\t%s"%(ofs, get_symbol_type(symbol), "None"))
            else:
                result[addr] = symb.name
                print("0x%0x\t%s\t%s"%(ofs, get_symbol_type(symbol), symbol.name))

    out_file.write(str(result))
    out_file.flush()
