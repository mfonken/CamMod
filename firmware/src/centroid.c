#include "centroid.h"

inline void cma( double new_val, double *avg, uint16_t num )
{
    *avg += ( new_val - *avg ) / ( num + 1 );
}

uint8_t getBlobId(double x, double y, uint16_t n_c, uint8_t *num_blobs)
{
    n_c /= 2;
    uint8_t id = NULL_C;                                         // NULL_ id (no adjacents)
    uint16_t i;
    for( i = 0; i < *num_blobs; i++ )
    {
        if( ( y - centroids.blobs[i].y_last ) <= MAX_GAP )      // Ensure blob hasn't expired
        {
            double x_l = (double)centroids.blobs[i].X;          // Current average
            double n_l = (double)( centroids.blobs[i].w_last ) / 2; // Last row width
        
            if( ( ( x + n_c + MAX_GAP)  >= ( x_l - n_l ) ) &&   // Check overlap of lower bound of blob and upper (with gap tolerance) of new
                ( ( x - n_c - MAX_GAP ) <= ( x_l + n_l ) ) )    // and of upper bound of blob and lower (with gap tolerance) of new
            {
                if( id == NULL_C )                               // If new blob is not claimed
                {
                    id = i;                                     // Claim it under current id
                }
                else
                {
                    cma( centroids.blobs[i].X, &centroids.blobs[id].X, 1);
                    cma( centroids.blobs[i].Y, &centroids.blobs[id].Y, 1);
                    centroids.blobs[id].mass += centroids.blobs[i].mass;
                    
                    uint8_t e = *num_blobs-1;
                    centroids.blobs[i] = centroids.blobs[e];
                    centroids.blobs[e].w_last = 0;
                    centroids.blobs[e].height = 0;
                    centroids.blobs[e].mass = 0;
                    ( *num_blobs )--;                           // Decrement blob counter
                }
            }
        }
    }
    return id;                                  // Return id: Valid if claimed, NULL_ if not
}

void getCentroids( uint8_t image_line[], uint16_t line_number )
{
    int16_t gap = NULL_G;
    uint16_t num_adj = 0;                     // Global variables
    double a_x_last = 0;                                                // Global last X and Y averages
    uint16_t x;
    for( x = 0; x < CENTROIDS_WIDTH; x += CENTROIDS_INTERVAL )          // Traverse all columns
    {
        uint8_t v = image_line[x];
        if( v > CENTROIDS_THRESH )                            // Check if pixel is on
        {
            gap = 0;                                                    // Reset gap counter
            cma( ( double )x, &a_x_last, num_adj );                     // Average adjacent pixels
            num_adj++;                                                  // Increment adjacent pixels
        }
        else if( gap != NULL_G )                                         // Otherwise, if gap counter is counting (i.e. there was a recent pixel
        {
            gap++;                                                      // Increment the gap counter
            if( gap >= MAX_GAP )                           // If max gap reached
            {                                                           // Include last pixel into a blob
                int16_t temp_id;
                temp_id = getBlobId( a_x_last, line_number, num_adj, &centroids.numBlobs );   // Get a blob to add to by coordinates and adjacent pixel width
                if( temp_id == NULL_C )                                  // If no blob return
                {
                    if( centroids.numBlobs <= MAX_BLOBS )               // Check if max blobs have already been filled
                    {
                        temp_id = centroids.numBlobs++;                     // Otherwise make a new id for the blob and increment the id counter
                    }
                }
                if( temp_id != NULL_C )
                {
                    cma(    a_x_last, &centroids.blobs[temp_id].X, centroids.blobs[temp_id].height ); // Cumulate the new pixels into the blob's X average
                    cma( line_number, &centroids.blobs[temp_id].Y, centroids.blobs[temp_id].height ); // Cumulate the new row into the blob's Y average
                    //centroids.blobs[temp_id].mass  += num_adj;
                    centroids.blobs[temp_id].w_last = num_adj;              // Update blob with: New last row width
                    centroids.blobs[temp_id].x_last = a_x_last;             // last row average
                    centroids.blobs[temp_id].height++;                      // height
                    centroids.blobs[temp_id].y_last = line_number;          // last row

                    num_adj = 0;                                            // Reset number of adjacent pixels
                    a_x_last = 0;                                           // Reset adjacent pixel average
                    gap = NULL_G;                                            // Reset the gap to NULL_
                }
            }
        }
    }
}

void initCentroids( uint16_t width, uint16_t height, uint16_t interval, uint8_t thresh )
{
    CENTROIDS_WIDTH     = width;
    CENTROIDS_HEIGHT    = height;
    CENTROIDS_INTERVAL  = interval;
    CENTROIDS_THRESH    = thresh;
    centroids.numBlobs  = 0;
}

void resetBlobs( void )
{
    int i;
    for( i = 0; i < centroids.numBlobs; i++ )
    {
        centroids.blobs[i].X = 0;
        centroids.blobs[i].Y = 0;
        centroids.blobs[i].mass   = 0;
        centroids.blobs[i].height = 0;
        centroids.blobs[i].w_last = 0;
        centroids.blobs[i].x_last = 0;
        centroids.blobs[i].y_last = 0;
    }
    centroids.numBlobs = 0;
}
