-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
--
-- $Id: rtpostgis_upgrade.sql.in.c 8448 2011-12-16 22:07:26Z dustymugs $
--
-- PostGIS Raster - Raster Type for PostGIS
-- http://trac.osgeo.org/postgis/wiki/WKTRaster
--
-- Copyright (c) 2011 Regina Obe <lr@pcorp.us>
-- Copyright (C) 2011 Regents of the University of California
--   <bkpark@ucdavis.edu>
--
-- This is free software; you can redistribute and/or modify it under
-- the terms of the GNU General Public Licence. See the COPYING file.
--
-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
--
-- WARNING: Any change in this file must be evaluated for compatibility.
--
-- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

-- This section is take add / drop things like CASTS, TYPES etc. that have changed
-- Since these are normally excluded from sed upgrade generator
-- they must be explicitly added
-- So that they can immediately be recreated. 
-- It is not run thru the sed processor to prevent it from being stripped
-- Note: We put these in separate file from drop since the extension module has
-- to add additional logic to drop them from the extension as well
#include "sqldefines.h"
/** -- GIST operator functions -- these don't seem necessary
DROP OPERATOR IF EXISTS << (raster, raster);
DROP FUNCTION IF EXISTS st_left(raster, raster);
DROP OPERATOR IF EXISTS &< (raster, raster);
DROP FUNCTION IF EXISTS st_overleft(raster, raster);
DROP OPERATOR IF EXISTS <<| (raster, raster);
DROP FUNCTION IF EXISTS st_below(raster, raster);
DROP OPERATOR IF EXISTS &<| (raster, raster);
DROP FUNCTION IF EXISTS st_overbelow(raster, raster);
DROP OPERATOR IF EXISTS && (raster, raster);
DROP FUNCTION IF EXISTS st_overlap(raster, raster);
DROP OPERATOR IF EXISTS &> (raster, raster);
DROP FUNCTION IF EXISTS st_overright(raster, raster);
DROP OPERATOR IF EXISTS >> (raster, raster);
DROP FUNCTION IF EXISTS st_right(raster, raster);
DROP OPERATOR IF EXISTS |&> (raster, raster);
DROP FUNCTION IF EXISTS st_overabove(raster, raster);
DROP OPERATOR IF EXISTS |>> (raster, raster);
DROP FUNCTION IF EXISTS st_above(raster, raster);
DROP OPERATOR IF EXISTS ~= (raster, raster);
DROP FUNCTION IF EXISTS st_same(raster, raster);
DROP OPERATOR IF EXISTS @ (raster, raster);
DROP FUNCTION IF EXISTS st_contained(raster, raster);
DROP OPERATOR IF EXISTS ~ (raster, raster);
DROP FUNCTION IF EXISTS st_contain(raster, raster); **/

-- drop st_bytea
DROP CAST IF EXISTS (raster AS bytea);
DROP FUNCTION IF EXISTS st_bytea(raster);

CREATE OR REPLACE FUNCTION bytea(raster)
    RETURNS bytea
    AS 'MODULE_PATHNAME', 'RASTER_to_bytea'
    LANGUAGE 'C' IMMUTABLE STRICT;
CREATE CAST (raster AS bytea)
    WITH FUNCTION bytea(raster) AS ASSIGNMENT;

-- drop box2d
DROP CAST IF EXISTS (raster AS box2d);
DROP FUNCTION IF EXISTS box2d(raster);

-- create box3d cast if it does not exist
#if POSTGIS_PGSQL_VERSION >= 90
-- If we are running 9.0+ we can use DO plpgsql to check
-- and only create if not exists so no need to force a drop
-- that way if people are using it, we will not mess them up
DO language 'plpgsql' $$DECLARE r record;
BEGIN
	IF NOT EXISTS(SELECT  cs.typname AS source
		FROM pg_cast AS ca 
        	INNER JOIN pg_type AS cs ON ca.castsource = cs.oid
        	INNER JOIN pg_type AS ct ON ca.casttarget = ct.oid
        	WHERE cs.typname = 'raster' AND ct.typname = 'box3d') THEN
		CREATE OR REPLACE FUNCTION box3d(raster)
		RETURNS box3d
		AS 'SELECT box3d(st_convexhull($1))'
		LANGUAGE 'SQL' IMMUTABLE STRICT;
		CREATE CAST (raster AS box3d)
			WITH FUNCTION box3d(raster) AS ASSIGNMENT;
    END IF;
END$$;	
#endif
#if POSTGIS_PGSQL_VERSION < 90
-- if we are running 8.4 we need to use brute force
DROP CAST IF EXISTS (raster AS box3d);
CREATE OR REPLACE FUNCTION box3d(raster)
    RETURNS box3d
    AS 'SELECT box3d(st_convexhull($1))'
    LANGUAGE 'SQL' IMMUTABLE STRICT;
CREATE CAST (raster AS box3d)
    WITH FUNCTION box3d(raster) AS ASSIGNMENT;
#endif

-- make geometry cast ASSIGNMENT
DROP CAST IF EXISTS (raster AS geometry);
CREATE CAST (raster AS geometry)
	WITH FUNCTION st_convexhull(raster) AS ASSIGNMENT;
