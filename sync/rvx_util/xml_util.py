from pathlib import Path
import xml.etree.ElementTree as XmlTree
from xml.dom import minidom

def read_xml_file(file_path:Path, tag:str=None):
  xml_tree = XmlTree.parse(str(file_path))
  xml_root = xml_tree.getroot()
  if tag:
    assert xml_root.tag==tag, f'tag mismatch {xml_root.tag}'
  return xml_root

def convert_xml_to_text(xml_root):
  xml_str = XmlTree.tostring(xml_root, encoding="unicode")
  xml_str = xml_str.replace('\n','')
  xml_str = xml_str.replace('\t','')
  xml_str = minidom.parseString(xml_str).toprettyxml(indent='  ')
  return xml_str

def xml2dict(xml_element):
  if len(xml_element):
    result = {}
    for xml_child in xml_element:
      previous = result.get(xml_child.tag)
      if not previous:
        result[xml_child.tag] = xml2dict(xml_child)
      elif type(previous)==list:
        result[xml_child.tag].append(xml2dict(xml_child))
      else:
        children_list = []
        children_list.append(previous)
        children_list.append(xml2dict(xml_child))
        result[xml_child.tag] = children_list
  else:
    result = xml_element.text
  return result

def print_xml(xml_element):
  print(convert_xml_to_text(xml_element))
