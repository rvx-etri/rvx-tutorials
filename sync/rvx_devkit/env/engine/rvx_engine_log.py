import datetime
from pathlib import Path

from rvx_engine_util import *

class RvxEngineJob():
  def __init__(self, name:str=None, is_local:bool=None, status:str=None):
    self.name = name
    self.is_local = is_local
    self.status = status
    self.is_file = False
    self.is_user = False
    self.log = ''

  @property
  def log_filename(self):
    return f'rvx_{self.name}.log'

  def set_status(self, status:str):
    self.status = status

  def add_log(self, log:str, is_file:bool, is_user:bool):
    assert not self.log, self.log
    self.is_file = is_file
    self.is_user = is_user
    self.log = log

  def import_from_xml(self, log_path:Path, xml_element:XmlTree.Element):
    self.name = xml_element.attrib['name']
    self.status = xml_element.attrib['status']
    self.is_local = xml_element.attrib['is_local']=='True'
    self.is_file = xml_element.attrib['is_file']=='True'
    self.is_user = xml_element.attrib['is_user']=='True'
    if self.is_file:
      self.log = (log_path/xml_element.text).read_text()
    else:
      self.log = xml_element.text
  
  def export_as_xml(self):
    xml_element = XmlTree.Element('rvx_job')
    xml_element.attrib['name'] = self.name
    xml_element.attrib['status'] = str(self.status)
    xml_element.attrib['is_local'] = str(self.is_local)
    xml_element.attrib['is_file'] = str(self.is_file)
    xml_element.attrib['is_user'] = str(self.is_user)
    if self.is_file:
      xml_element.text = self.log_filename
    else:
      xml_element.text = self.log
    return xml_element

  def __repr__(self):
    if self.status=='error':
      text = f'[ERROR] {self.log}'
    else:
      text = str(self.log)
    return text

class RvxEngineLog():
  pass

class RvxEngineLog():
  def __init__(self, log_dir=None):
    if log_dir:
      self.log_path = Path(log_dir).resolve()
      assert self.log_path.is_dir(), self.log_path
    else:
      self.log_path = None
    self.__clear()

  @property
  def log_file(self):
    return self.log_path / 'rvx_engine.log'

  @property
  def current_job(self):
    assert self.job_list
    return self.job_list[-1]

  @property
  def current_log(self):
    return self.current_job.log if self.current_job else None

  @property
  def log_tar_file(self):
    return self.log_path / 'rvx_engine_log.tar.gz'

  @property
  def cloud_log_file(self):
    return self.log_path / self.cloud_log_filename

  def __clear(self):
    self.date = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    self.job_list = []

  def import_file(self, log_path:Path):
    assert log_path.is_dir(), log_path
    self.log_path = log_path
    self.__clear()
    xml_root = read_xml_file(self.log_file)
    for xml_element in xml_root:
      if xml_element.tag=='date':
        self.date = xml_element.text
      else:
        job = RvxEngineJob()
        job.import_from_xml(self.log_path, xml_element)
        self.job_list.append(job)
  
  def append_job(self, job:RvxEngineJob):
    self.job_list.append(job)

  def add_new_job(self, job_name:str, is_local:bool, job_status:str=None):
    self.job_list.append(RvxEngineJob(job_name,is_local,job_status))
    return self.current_job

  def set_status(self, status:str):
    self.current_job.set_status(status)
  
  def add_log(self, log:str, is_file:bool, is_user:bool):
    self.current_job.add_log(log, is_file, is_user)

  def export_as_xml(self):
    xml_root = XmlTree.Element('rvx_log')
    xml_element = XmlTree.SubElement(xml_root, 'date')
    xml_element.text = self.date
    for job in self.job_list:
      xml_root.append(job.export_as_xml())
    return xml_root

  def export_file(self, compress:bool=False):
    for job in self.job_list:
      if job.is_file:
        specific_log_file = self.log_path / job.log_filename
        remove_file(specific_log_file)
        specific_log_file.write_text(job.log)
    
    xml_root = self.export_as_xml()
    xml_str = convert_xml_to_text(xml_root)
    self.log_file.write_text(xml_str)

    if compress:
      execute_shell_cmd(f'tar -czf {self.log_tar_file} ./rvx_*.log', self.log_path)

  def clean(self):
    remove_files(self.log_path, 'rvx_*.log')
    self.__clear()

  def __repr__(self):
    return convert_xml_to_text(self.export_as_xml())
