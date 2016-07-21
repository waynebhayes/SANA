How to balance sequence in an alpha-style way:

--custom ../../<file here> auto similarity clamp <weight of sequence> 0 -c pairWeightGED=<weight of topology> -c pairWeightGraphlets=0

The ../../ is necessary to redirect from wrappedAlgorithms/NewGEDEVO/ to the main sana directory. Also make sure to respecify the max time if using wrappedArgs, either --maxsecs, --maxiter, or --maxsame
