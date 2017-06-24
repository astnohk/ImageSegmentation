# segmentation

This is simple image segmentation program.
It processing the input images with mean shift algorithm.


## Mean shift

The mean shift in this program is bit altered to defeat sprinkling very small clusters in textural regions which may make very large and undesirous region in regions connecting phase.
To do that the algorithm process mean shift and small regions whose pixels is under M (pre-determined constant) and whose all neighbors pixels are also under M then the regions concatenate with its all neighbors.
The second condition will avoid to make undesirous very long and thin region such as connected edges of objects.
It will NOT affect on true thin objects because in almost all cases the objects has same color in the region.

