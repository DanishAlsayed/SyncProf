# SyncProf
LLVM pass that generates time profiles  for Synchronization functions.

My final year project under the supervision of Dr. Heming Cui at The University of Hong Kong. SyncProf employes techniques from Continuously Measuring Critical Section Pressure with
the Free Lunch Profiler (Florian David, Ga¨el Thomas, Julia Lawall, Gilles Muller) like critical section pressure (CSP).

To compile SyncProf.cpp in to SyncProf.so just run the "make" command and make sure to include and compile rtlib.cc in your programs source code and include the generates rtlib.o where ever needed as SyncProf instruments program code with functions from rtlib.cc.

Feel free to contact me for any questions/suggestions/contribution.
