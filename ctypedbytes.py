# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

def classes():

    import sys
    import typedbytes
    from fastb import reads, writes

    
    def flatten(iterable):
        for i in iterable:
            for j in i:
                yield j


    class Input(typedbytes.Input):

        def reads(self):
            return reads(self.file, self.lookup)


    class Output(typedbytes.Output):

        def writes(self, iterable):
            writes(self.file, iterable, self.lookup)

    
    class PairedInput(typedbytes.PairedInput):

        def reads(self):
            next = reads(self.file, self.lookup).next
            while True:
                key = next()
                try:
                    value = next()
                except StopIteration:
                    raise StructError('EOF before second item in pair')
                yield key, value

  
    class PairedOutput(typedbytes.PairedOutput):
 
        def writes(self, iterable):
            writes(self.file, flatten(iterable), self.lookup)


    return Input, Output, PairedInput, PairedOutput, typedbytes.Bytes
 

Input, Output, PairedInput, PairedOutput, Bytes = classes()
