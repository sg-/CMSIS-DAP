"""
mbed SDK
Copyright (c) 2011-2013 ARM Limited

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""
from os.path import basename


class Uvision4():
    NAME = 'uVision4'

    def generate(self, cpu):
        # target = self.target.lower()

        # Project file
        self.gen_file('uvision4_%s.uvproj.tmpl' % target, ctx, '%s.uvproj' % self.program_name)
        self.gen_file('uvision4_%s.uvopt.tmpl' % target, ctx, '%s.uvopt' % self.program_name)
