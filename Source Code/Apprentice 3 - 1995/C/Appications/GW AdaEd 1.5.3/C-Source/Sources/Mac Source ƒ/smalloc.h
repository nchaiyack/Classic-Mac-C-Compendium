/* smallocp.h - used to determine whether to use smalloc or malloc
 *
 * This was in config.h, which made the whole project dependent on
 * this definition, although only three files used this definition.
 * For the mac, I needed to play around with this definition, so I
 * moved it out here, and included this file in the source files that
 * used this symbol.
 *
 * manuel
 */

/* define SMALLOC to use smalloc for string allocation */
#define SMALLOC

