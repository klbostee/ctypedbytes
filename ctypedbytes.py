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
            return reads(self.file, self._read)


    class Output(typedbytes.Output):

        def writes(self, iterable):
            writes(self.file, iterable, self._write)

    
    class PairedInput(typedbytes.PairedInput):

        def reads(self):
            next = reads(self.file, self._read).next
            while True:
                key = next()
                try:
                    value = next()
                except StopIteration:
                    raise StructError('EOF before second item in pair')
                yield key, value

  
    class PairedOutput(typedbytes.PairedOutput):
 
        def writes(self, iterable):
            writes(self.file, flatten(iterable), self._write)


    return Input, Output, PairedInput, PairedOutput
 

Input, Output, PairedInput, PairedOutput = classes()
