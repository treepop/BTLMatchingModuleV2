/* This is a config file that was used in both extractDesc and matchingDesc project. */
#define CENTER 99 // Radius of a center pixel that will be used to calculate HSV histogram.
				  // Max = 99.

/* Number of vectors that lowest distance of Surf feature. */
#define NUMTOPSMALL 10 // Max = 8.

/* Set FEATURE to 1 for rank from color feature only.
 * 2 for rank from shape feature only.
 * 3 for use both color and shape feature.
 */
#define FEATURE 3

/* If the answer was in TOPN, it indicate that correct. */
#define TOPN 5

/* Algorithms for performance measurement.
 * 1 for weight equal.
 * 2 for weight vary.
 */
#define PERFMEA 2