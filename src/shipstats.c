/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file shipstats.c
 *
 * @brief Handles the ship statistics.
 */


#include "shipstats.h"

#include "naev.h"

#include "log.h"


typedef struct ShipStatsLookup_ {
   ShipStatsType type;
   const char *name;
   size_t offset;
   int data;
} ShipStatsLookup;


#define ELEM( t, n, d ) \
   { .type=t, .name="#n", .offset=offsetof( ShipStats, n ), .data=d }
#define NELEM( t ) \
   { .type=t, .name=NULL, .offset=0 }

static const ShipStatsLookup ss_lookup[] = {
   NELEM( SS_TYPE_NIL ),

   ELEM( SS_TYPE_D_JUMP_DELAY,   jump_delay, 0 ),
   ELEM( SS_TYPE_D_JUMP_RANGE,   jump_range, 0 ),
   ELEM( SS_TYPE_D_CARGO_INERTIA, cargo_inertia, 0 ),

   ELEM( SS_TYPE_D_EW_HIDE,      ew_hide, 0 ),
   ELEM( SS_TYPE_D_EW_DETECT,    ew_detect, 0 ),

   ELEM( SS_TYPE_D_LAUNCH_RATE,  launch_rate, 0 ),
   ELEM( SS_TYPE_D_LAUNCH_RANGE, launch_range, 0 ),
   ELEM( SS_TYPE_D_AMMO_CAPACITY, ammo_capacity, 0 ),

   ELEM( SS_TYPE_D_FORWARD_HEAT, fwd_heat, 0 ),
   ELEM( SS_TYPE_D_FORWARD_DAMAGE, fwd_damage, 0 ),
   ELEM( SS_TYPE_D_FORWARD_FIRERATE, fwd_firerate, 0 ),
   ELEM( SS_TYPE_D_FORWARD_ENERGY, fwd_energy, 0 ),

   ELEM( SS_TYPE_D_TURRET_HEAT, tur_heat, 0 ),
   ELEM( SS_TYPE_D_TURRET_DAMAGE, tur_damage, 0 ),
   ELEM( SS_TYPE_D_TURRET_FIRERATE, tur_firerate, 0 ),
   ELEM( SS_TYPE_D_TURRET_ENERGY, tur_energy, 0 ),

   ELEM( SS_TYPE_D_NEBULA_DMG_SHIELD, nebula_dmg_shield, 0 ),
   ELEM( SS_TYPE_D_NEBULA_DMG_ARMOUR, nebula_dmg_armour, 0 ),

   ELEM( SS_TYPE_D_HEAT_DISSIPATION, heat_dissipation, 0 ),

   NELEM( SS_TYPE_D_SENTINAL ),

   NELEM( SS_TYPE_B_SENTINAL ),

   /* Sentinal. */
   NELEM( SS_TYPE_SENTINAL )
};

#undef NELEM
#undef ELEM


/*
 * Prototypes.
 */
static int ss_statsMod( ShipStats *stats, const ShipStatList* list );


/**
 * @brief Creatse a shipstat list element from an xml node.
 *
 *    @param node Node to create element from.
 *    @return Liste lement created from node.
 */
ShipStatList* ss_listFromXML( xmlNodePtr node )
{
   const ShipStatsLookup *sl;
   ShipStatList *ll;
   ShipStatsType type;

   /* Try to get type. */
   type = ss_typeFromName( (char*) node->name );
   if (type == SS_TYPE_NIL)
      return NULL;

   /* Allocate. */
   ll = malloc( sizeof(ShipStatList) );
   ll->next    = NULL;
   ll->target  = 0;
   ll->type    = type;

   /* Set the data. */
   sl = &ss_lookup[ type ];
   if (sl->type == 0)
      ll->d.d     = xml_getFloat(node);

   return ll;
}


/**
 * @brief Checks for sanity.
 */
int ss_check (void)
{
   ShipStatsType i;

   for (i=0; i<=SS_TYPE_SENTINAL; i++) {
      if (ss_lookup[i].type != i) {
         WARN("ss_lookup: %s should have id %d but has %d",
               ss_lookup[i].name, i, ss_lookup[i].type );
         return -1;
      }
   }

   return 0;
}


/**
 * @brief Initializes a stat structure.
 */
int ss_statsInit( ShipStats *stats )
{
   int i;
   char *ptr;
   double *dbl;
   const ShipStatsLookup *sl;

#if DEBUGGING
   memset( stats, 0, sizeof(ShipStats) );
#endif /* DEBUGGING */

   ptr = (char*) stats;
   for (i=0; i<SS_TYPE_SENTINAL; i++) {
      sl = &ss_lookup[ i ];

      /* Only want valid names. */
      if (sl->name == NULL)
         continue;

      /* Handle doubles. */
      if (sl->type == 0) {
         dbl   = (double*) &ptr[ sl->offset ];
         *dbl  = 1.0;
      }
   }

   return 0;
}


/**
 * @brief Modifies a stat structure using a single element.
 *
 *    @param stats Stat structure to modify.
 *    @param list Single element to apply.
 *    @return 0 on success.
 */
static int ss_statsMod( ShipStats *stats, const ShipStatList* list )
{
   char *ptr;
   double *dbl;
   const ShipStatsLookup *sl = &ss_lookup[ list->type ];

   ptr = (char*) stats;
   if (sl->data == 0) {
      dbl   = (double*) &ptr[ sl->offset ];
      *dbl *= list->d.d;
   }

   return 0;
}


/**
 * @brief Updates a stat structure from a stat list.
 *
 *    @param stats Stats to update.
 *    @param list List to update from.
 */
int ss_statsModFromList( ShipStats *stats, const ShipStatList* list )
{
   int ret;
   const ShipStatList *ll;

   ret = 0;
   for (ll = list; ll != NULL; ll = ll->next)
      ret |= ss_statsMod( stats, ll );

   return ret;
}


/**
 * @brief Gets the name from type.
 *
 * O(1) look up.
 *
 *    @param type Type to get name of.
 *    @return Name of the type.
 */
const char* ss_nameFromType( ShipStatsType type )
{
   return ss_lookup[ type ].name;
}


/**
 * @brief Gets the type from the name.
 *
 *    @param name Name to get type of.
 *    @return Type matching the name.
 */
ShipStatsType ss_typeFromName( const char *name )
{
   int i;
   for (i=0; ss_lookup[i].type != SS_TYPE_SENTINAL; i++)
      if (strcmp(name,ss_lookup[i].name)==0)
         return ss_lookup[i].type;
   return SS_TYPE_NIL;
}



