Semi-transactional file support. 

Overview
---------
During processing data is written to temporary output file. 
If failure occurs during processing, temporary file can be discarded.
If processing is completed, temporary file replaces original file on "commit" operation.

