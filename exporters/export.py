from optparse import OptionParser
import yaml
import logging
import os
from yaml_parser import parse_yaml, get_project_files, parse_list_yaml, get_ide
from uvision4 import Uvision4
import sys
from os.path import basename

def run_generator(dic, project):
    project_list = []
    yaml_files = get_project_files(dic, project) # TODO fix list inside list
    if yaml_files:
        for yaml_file in yaml_files:
            try:
                file = open(yaml_file)
            except IOError:
                print "Cannot open a file: %s" % yaml_file
                sys.exit()
            else:
                loaded_yaml = yaml.load(file)
                project_list.append(parse_yaml(loaded_yaml))
                file.close()
        process_data = parse_list_yaml(project_list)
    else:
        print "Project record is empty"
        sys.exit()

    ide = get_ide(process_data)
    if ide is None:
        print "No toolchain selected in records. Exiting"
        sys.exit()

    logging.info("Generating project: %s" % project)
    exporter = Uvision4()
    #run exporter for defined bootloader project
    exporter.generate(process_data)

def process_all_projects(dic):
    projects = []
    yaml_files = []
    for k,v in dic['projects'].items():
        projects.append(k);

    for project in projects:
        run_generator(dic, project)

def process_project(dic, project):
    run_generator(dic, project)

if __name__ == '__main__':
    logging.basicConfig(level=logging.DEBUG)

    # Parse Options
    parser = OptionParser()
    parser.add_option("-f", "--file", help="Yaml projects file")
    parser.add_option("-p", "--project", help="Project to be generated")

    (options, args) = parser.parse_args()

    if not options.file:
        print "No project file give. Please provide one. Exiting"
        sys.exit()

    # always run from the root directory
    script_dir = os.path.dirname(os.path.realpath(sys.argv[0]))
    os.chdir(script_dir)
    os.chdir('../')

    print "Processing projects file."
    project_file = open(options.file)
    config = yaml.load(project_file)

    if options.project:
        process_project(config, options.project) # one project
    else:
        process_all_projects(config) # all projects within project.yaml

    project_file.close()
