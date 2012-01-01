-- $Id$
-- Legacy functions without chip functions --
#include "legacy_compatibility_layer.sql.in.c"
--- start functions that in theory should never have been used or internal like stuff deprecated

-- these were superceded by PostGIS_AddBBOX , PostGIS_DropBBOX, PostGIS_HasBBOX in 1.5 --
CREATE OR REPLACE FUNCTION addbbox(geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME','LWGEOM_addBBOX'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
CREATE OR REPLACE FUNCTION dropbbox(geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME','LWGEOM_dropBBOX'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION hasbbox(geometry)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'LWGEOM_hasBBOX'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION noop(geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_noop'
	LANGUAGE 'C' VOLATILE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION SetSRID(geometry,int4)
	RETURNS geometry
	AS 'MODULE_PATHNAME','LWGEOM_set_srid'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
--- Deprecation in 1.5.0
CREATE OR REPLACE FUNCTION st_box2d(geometry)
	RETURNS box2d
	AS 'MODULE_PATHNAME','LWGEOM_to_BOX2DFLOAT4'
	LANGUAGE 'C' IMMUTABLE STRICT;

--- Deprecation in 1.5.0
CREATE OR REPLACE FUNCTION st_box3d(geometry)
	RETURNS box3d
	AS 'MODULE_PATHNAME','LWGEOM_to_BOX3D'
	LANGUAGE 'C' IMMUTABLE STRICT;

--- Deprecation in 1.5.0
CREATE OR REPLACE FUNCTION st_box(geometry)
	RETURNS box
	AS 'MODULE_PATHNAME','LWGEOM_to_BOX'
	LANGUAGE 'C' IMMUTABLE STRICT;

--- Deprecation in 1.5.0
CREATE OR REPLACE FUNCTION st_box2d(box3d)
	RETURNS box2d
	AS 'MODULE_PATHNAME','BOX3D_to_BOX2DFLOAT4'
	LANGUAGE 'C' IMMUTABLE STRICT;

--- Deprecation in 1.5.0
CREATE OR REPLACE FUNCTION st_box3d(box2d)
	RETURNS box3d
	AS 'MODULE_PATHNAME','BOX2DFLOAT4_to_BOX3D'
	LANGUAGE 'C' IMMUTABLE STRICT;

--- Deprecation in 1.5.0
CREATE OR REPLACE FUNCTION st_box(box3d)
	RETURNS box
	AS 'MODULE_PATHNAME','BOX3D_to_BOX'
	LANGUAGE 'C' IMMUTABLE STRICT;

--- Deprecation in 1.5.0
CREATE OR REPLACE FUNCTION st_text(geometry)
	RETURNS text
	AS 'MODULE_PATHNAME','LWGEOM_to_text'
	LANGUAGE 'C' IMMUTABLE STRICT;

--- Deprecation in 1.5.0
CREATE OR REPLACE FUNCTION st_geometry(box2d)
	RETURNS geometry
	AS 'MODULE_PATHNAME','BOX2DFLOAT4_to_LWGEOM'
	LANGUAGE 'C' IMMUTABLE STRICT;

--- Deprecation in 1.5.0
CREATE OR REPLACE FUNCTION st_geometry(box3d)
	RETURNS geometry
	AS 'MODULE_PATHNAME','BOX3D_to_LWGEOM'
	LANGUAGE 'C' IMMUTABLE STRICT;

--- Deprecation in 1.5.0
CREATE OR REPLACE FUNCTION st_geometry(text)
	RETURNS geometry
	AS 'MODULE_PATHNAME','parse_WKT_lwgeom'
	LANGUAGE 'C' IMMUTABLE STRICT;

--- Deprecation in 1.5.0
CREATE OR REPLACE FUNCTION st_geometry(bytea)
	RETURNS geometry
	AS 'MODULE_PATHNAME','LWGEOM_from_bytea'
	LANGUAGE 'C' IMMUTABLE STRICT;

--- Deprecation in 1.5.0
CREATE OR REPLACE FUNCTION st_bytea(geometry)
	RETURNS bytea
	AS 'MODULE_PATHNAME','LWGEOM_to_bytea'
	LANGUAGE 'C' IMMUTABLE STRICT;

-- Deprecation in 1.5.0
CREATE OR REPLACE FUNCTION st_box3d_in(cstring)
	RETURNS box3d
	AS 'MODULE_PATHNAME', 'BOX3D_in'
	LANGUAGE 'C' IMMUTABLE STRICT;

-- Deprecation in 1.5.0
CREATE OR REPLACE FUNCTION st_box3d_out(box3d)
	RETURNS cstring
	AS 'MODULE_PATHNAME', 'BOX3D_out'
	LANGUAGE 'C' IMMUTABLE STRICT;

-- START MANAGEMENT FUNCTIONS
-- These are legacy management functions with no place in our 2.0 world
-----------------------------------------------------------------------
-- RENAME_GEOMETRY_TABLE_CONSTRAINTS()
-----------------------------------------------------------------------
-- This function has been obsoleted for the difficulty in
-- finding attribute on which the constraint is applied.
-- AddGeometryColumn will name the constraints in a meaningful
-- way, but nobody can rely on it since old postgis versions did
-- not do that.
-----------------------------------------------------------------------
CREATE OR REPLACE FUNCTION rename_geometry_table_constraints() RETURNS text
AS
$$
SELECT 'rename_geometry_table_constraint() is obsoleted'::text
$$
LANGUAGE 'SQL' IMMUTABLE;

-----------------------------------------------------------------------
-- FIX_GEOMETRY_COLUMNS()
-----------------------------------------------------------------------
-- This function will:
--
--	o try to fix the schema of records with an integer one
--		(for PG>=73)
--
--	o link records to system tables through attrelid and varattnum
--		(for PG<75)
--
--	o delete all records for which no linking was possible
--		(for PG<75)
--
--
-----------------------------------------------------------------------
CREATE OR REPLACE FUNCTION fix_geometry_columns() RETURNS text
AS
$$
DECLARE
	mislinked record;
	result text;
	linked integer;
	deleted integer;
	foundschema integer;
BEGIN

	-- Since 7.3 schema support has been added.
	-- Previous postgis versions used to put the database name in
	-- the schema column. This needs to be fixed, so we try to
	-- set the correct schema for each geometry_colums record
	-- looking at table, column, type and srid.
	/** UPDATE geometry_columns SET f_table_schema = n.nspname
		FROM pg_namespace n, pg_class c, pg_attribute a,
			pg_constraint sridcheck, pg_constraint typecheck
			WHERE ( f_table_schema is NULL
		OR f_table_schema = ''
			OR f_table_schema NOT IN (
					SELECT nspname::varchar
					FROM pg_namespace nn, pg_class cc, pg_attribute aa
					WHERE cc.relnamespace = nn.oid
					AND cc.relname = f_table_name::name
					AND aa.attrelid = cc.oid
					AND aa.attname = f_geometry_column::name))
			AND f_table_name::name = c.relname
			AND c.oid = a.attrelid
			AND c.relnamespace = n.oid
			AND f_geometry_column::name = a.attname

			AND sridcheck.conrelid = c.oid
		AND sridcheck.consrc LIKE '(%srid(% = %)'
			AND sridcheck.consrc ~ textcat(' = ', srid::text)

			AND typecheck.conrelid = c.oid
		AND typecheck.consrc LIKE
		'((geometrytype(%) = ''%''::text) OR (% IS NULL))'
			AND typecheck.consrc ~ textcat(' = ''', type::text)

			AND NOT EXISTS (
					SELECT oid FROM geometry_columns gc
					WHERE c.relname::varchar = gc.f_table_name
					AND n.nspname::varchar = gc.f_table_schema
					AND a.attname::varchar = gc.f_geometry_column
			);

	GET DIAGNOSTICS foundschema = ROW_COUNT; 

	-- no linkage to system table needed
	return 'fixed:'||foundschema::text; **/
	return 'This function is obsolete now that geometry_columns is a view';

END;
$$
LANGUAGE 'plpgsql' VOLATILE;

-----------------------------------------------------------------------
-- PROBE_GEOMETRY_COLUMNS()
-----------------------------------------------------------------------
-- Fill the geometry_columns table with values probed from the system
-- catalogues. This is done by simply looking up constraints previously
-- added to a geometry column. If geometry constraints are missing, no
-- attempt is made to add the necessary constraints to the geometry
-- column, nor is it recorded in the geometry_columns table.
-- 3d flag cannot be probed, it defaults to 2
--
-- Note that bogus records already in geometry_columns are not
-- overridden (a check for schema.table.column is performed), so
-- to have a fresh probe backup your geometry_columns, delete from
-- it and probe.
-----------------------------------------------------------------------
CREATE OR REPLACE FUNCTION probe_geometry_columns() RETURNS text AS
$$
DECLARE
	inserted integer;
	oldcount integer;
	probed integer;
	stale integer;
BEGIN

/*	SELECT count(*) INTO oldcount FROM geometry_columns;

	SELECT count(*) INTO probed
		FROM pg_class c, pg_attribute a, pg_type t,
			pg_namespace n,
			pg_constraint sridcheck, pg_constraint typecheck

		WHERE t.typname = 'geometry'
		AND a.atttypid = t.oid
		AND a.attrelid = c.oid
		AND c.relnamespace = n.oid
		AND sridcheck.connamespace = n.oid
		AND typecheck.connamespace = n.oid
		AND sridcheck.conrelid = c.oid
		AND sridcheck.consrc LIKE '(%srid('||a.attname||') = %)'
		AND typecheck.conrelid = c.oid
		AND typecheck.consrc LIKE
		'((geometrytype('||a.attname||') = ''%''::text) OR (% IS NULL))'
		;

	INSERT INTO geometry_columns SELECT
		''::varchar as f_table_catalogue,
		n.nspname::varchar as f_table_schema,
		c.relname::varchar as f_table_name,
		a.attname::varchar as f_geometry_column,
		2 as coord_dimension,
		trim(both  ' =)' from
			replace(replace(split_part(
				sridcheck.consrc, ' = ', 2), ')', ''), '(', ''))::integer AS srid,
		trim(both ' =)''' from substr(typecheck.consrc,
			strpos(typecheck.consrc, '='),
			strpos(typecheck.consrc, '::')-
			strpos(typecheck.consrc, '=')
			))::varchar as type
		FROM pg_class c, pg_attribute a, pg_type t,
			pg_namespace n,
			pg_constraint sridcheck, pg_constraint typecheck
		WHERE t.typname = 'geometry'
		AND a.atttypid = t.oid
		AND a.attrelid = c.oid
		AND c.relnamespace = n.oid
		AND sridcheck.connamespace = n.oid
		AND typecheck.connamespace = n.oid
		AND sridcheck.conrelid = c.oid
		AND sridcheck.consrc LIKE '(%srid('||a.attname||') = %)'
		AND typecheck.conrelid = c.oid
		AND typecheck.consrc LIKE
		'((geometrytype('||a.attname||') = ''%''::text) OR (% IS NULL))'

			AND NOT EXISTS (
					SELECT oid FROM geometry_columns gc
					WHERE c.relname::varchar = gc.f_table_name
					AND n.nspname::varchar = gc.f_table_schema
					AND a.attname::varchar = gc.f_geometry_column
			);

	GET DIAGNOSTICS inserted = ROW_COUNT;

	IF oldcount > probed THEN
		stale = oldcount-probed;
	ELSE
		stale = 0;
	END IF;

	RETURN 'probed:'||probed::text||
		' inserted:'||inserted::text||
		' conflicts:'||(probed-inserted)::text||
		' stale:'||stale::text;*/
	RETURN 'This function is obsolete now that geometry_columns is a view';
END

$$
LANGUAGE 'plpgsql' VOLATILE;



-- END MANAGEMENT FUNCTIONS --

-- Deprecation in 1.5.0
-- these remarked out functions cause problems and no one uses them directly
-- They should not be installed
/** CREATE OR REPLACE FUNCTION st_geometry_analyze(internal)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'geometry_analyze'
	LANGUAGE 'C' VOLATILE STRICT;
	
-- Deprecation in 1.5.0
CREATE OR REPLACE FUNCTION st_geometry_in(cstring)
	RETURNS geometry
	AS 'MODULE_PATHNAME','LWGEOM_in'
	LANGUAGE 'C' IMMUTABLE STRICT;

-- Deprecation in 1.5.0
CREATE OR REPLACE FUNCTION st_geometry_out(geometry)
	RETURNS cstring
	AS 'MODULE_PATHNAME','LWGEOM_out'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.5.0
CREATE OR REPLACE FUNCTION st_geometry_recv(internal)
	RETURNS geometry
	AS 'MODULE_PATHNAME','LWGEOM_recv'
	LANGUAGE 'C' IMMUTABLE STRICT;

-- Deprecation in 1.5.0
CREATE OR REPLACE FUNCTION st_geometry_send(geometry)
	RETURNS bytea
	AS 'MODULE_PATHNAME','LWGEOM_send'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.5.0
CREATE OR REPLACE FUNCTION st_spheroid_in(cstring)
	RETURNS spheroid
	AS 'MODULE_PATHNAME','ellipsoid_in'
	LANGUAGE 'C' IMMUTABLE STRICT;

-- Deprecation in 1.5.0
CREATE OR REPLACE FUNCTION st_spheroid_out(spheroid)
	RETURNS cstring
	AS 'MODULE_PATHNAME','ellipsoid_out'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
**/
-- Deprecation in 1.5.0
CREATE OR REPLACE FUNCTION st_geometry_lt(geometry, geometry)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'lwgeom_lt'
	LANGUAGE 'C' IMMUTABLE STRICT;

-- Deprecation in 1.5.0
CREATE OR REPLACE FUNCTION st_geometry_le(geometry, geometry)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'lwgeom_le'
	LANGUAGE 'C' IMMUTABLE STRICT;

-- Deprecation in 1.5.0
CREATE OR REPLACE FUNCTION st_geometry_gt(geometry, geometry)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'lwgeom_gt'
	LANGUAGE 'C' IMMUTABLE STRICT;

-- Deprecation in 1.5.0
CREATE OR REPLACE FUNCTION st_geometry_ge(geometry, geometry)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'lwgeom_ge'
	LANGUAGE 'C' IMMUTABLE STRICT;

-- Deprecation in 1.5.0
CREATE OR REPLACE FUNCTION st_geometry_eq(geometry, geometry)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'lwgeom_eq'
	LANGUAGE 'C' IMMUTABLE STRICT;

-- Deprecation in 1.5.0
CREATE OR REPLACE FUNCTION st_geometry_cmp(geometry, geometry)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'lwgeom_cmp'
	LANGUAGE 'C' IMMUTABLE STRICT;
	

--- end functions that in theory should never have been used


-- begin old ogc (and non-ST) names that have been replaced with new SQL-MM and SQL ST_ Like names --
-- AFFINE Functions --
-- Availability: 1.1.2
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION Affine(geometry,float8,float8,float8,float8,float8,float8,float8,float8,float8,float8,float8,float8)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_affine'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Availability: 1.1.2
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION Affine(geometry,float8,float8,float8,float8,float8,float8)
	RETURNS geometry
	AS 'SELECT st_affine($1,  $2, $3, 0,  $4, $5, 0,  0, 0, 1,  $6, $7, 0)'
	LANGUAGE 'SQL' IMMUTABLE STRICT;

-- Availability: 1.1.2
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION RotateZ(geometry,float8)
	RETURNS geometry
	AS 'SELECT st_affine($1,  cos($2), -sin($2), 0,  sin($2), cos($2), 0,  0, 0, 1,  0, 0, 0)'
	LANGUAGE 'SQL' IMMUTABLE STRICT;

-- Availability: 1.1.2
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION Rotate(geometry,float8)
	RETURNS geometry
	AS 'SELECT st_rotateZ($1, $2)'
	LANGUAGE 'SQL' IMMUTABLE STRICT;

-- Availability: 1.1.2
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION RotateX(geometry,float8)
	RETURNS geometry
	AS 'SELECT st_affine($1, 1, 0, 0, 0, cos($2), -sin($2), 0, sin($2), cos($2), 0, 0, 0)'
	LANGUAGE 'SQL' IMMUTABLE STRICT;

-- Availability: 1.1.2
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION RotateY(geometry,float8)
	RETURNS geometry
	AS 'SELECT st_affine($1,  cos($2), 0, sin($2),  0, 1, 0,  -sin($2), 0, cos($2), 0,  0, 0)'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Availability: 1.1.0
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION Scale(geometry,float8,float8,float8)
	RETURNS geometry
	AS 'SELECT st_affine($1,  $2, 0, 0,  0, $3, 0,  0, 0, $4,  0, 0, 0)'
	LANGUAGE 'SQL' IMMUTABLE STRICT;

-- Availability: 1.1.0
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION Scale(geometry,float8,float8)
	RETURNS geometry
	AS 'SELECT st_scale($1, $2, $3, 1)'
	LANGUAGE 'SQL' IMMUTABLE STRICT;

-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION Translate(geometry,float8,float8,float8)
	RETURNS geometry
	AS 'SELECT st_affine($1, 1, 0, 0, 0, 1, 0, 0, 0, 1, $2, $3, $4)'
	LANGUAGE 'SQL' IMMUTABLE STRICT;

-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION Translate(geometry,float8,float8)
	RETURNS geometry
	AS 'SELECT st_translate($1, $2, $3, 0)'
	LANGUAGE 'SQL' IMMUTABLE STRICT;


-- Availability: 1.1.0
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION TransScale(geometry,float8,float8,float8,float8)
	RETURNS geometry
	AS 'SELECT st_affine($1,  $4, 0, 0,  0, $5, 0,
		0, 0, 1,  $2 * $4, $3 * $5, 0)'
	LANGUAGE 'SQL' IMMUTABLE STRICT;

-- END Affine functions
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION AddPoint(geometry, geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_addpoint'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION AddPoint(geometry, geometry, integer)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_addpoint'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION Area(geometry)
	RETURNS FLOAT8
	AS 'MODULE_PATHNAME','LWGEOM_area_polygon'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- this is an alias for 'area(geometry)'
-- there is nothing such an 'area3d'...
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION Area2D(geometry)
	RETURNS FLOAT8
	AS 'MODULE_PATHNAME', 'LWGEOM_area_polygon'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION AsEWKB(geometry)
	RETURNS BYTEA
	AS 'MODULE_PATHNAME','WKBFromLWGEOM'
	LANGUAGE 'C' IMMUTABLE STRICT;
	

-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION AsEWKB(geometry,text)
	RETURNS bytea
	AS 'MODULE_PATHNAME','WKBFromLWGEOM'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION AsEWKT(geometry)
	RETURNS TEXT
	AS 'MODULE_PATHNAME','LWGEOM_asEWKT'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- AsGML(geom) / precision=15 version=2
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION AsGML(geometry)
	RETURNS TEXT
	AS 'SELECT _ST_AsGML(2, $1, 15, 0, null)'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION AsGML(geometry, int4)
	RETURNS TEXT
	AS 'SELECT _ST_AsGML(2, $1, $2, 0, null)'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- AsKML(geom, precision) / version=2
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION AsKML(geometry, int4)
	RETURNS TEXT
	AS 'SELECT _ST_AsKML(2, ST_transform($1,4326), $2, null)'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- AsKML(geom) / precision=15 version=2
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION AsKML(geometry)
	RETURNS TEXT
	AS 'SELECT _ST_AsKML(2, ST_Transform($1,4326), 15, null)'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- AsKML(version, geom, precision)
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION AsKML(int4, geometry, int4)
	RETURNS TEXT
	AS 'SELECT _ST_AsKML($1, ST_Transform($2,4326), $3, null)'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION AsHEXEWKB(geometry)
	RETURNS TEXT
	AS 'MODULE_PATHNAME','LWGEOM_asHEXEWKB'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION AsHEXEWKB(geometry, text)
	RETURNS TEXT
	AS 'MODULE_PATHNAME','LWGEOM_asHEXEWKB'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION AsSVG(geometry)
	RETURNS TEXT
	AS 'MODULE_PATHNAME','LWGEOM_asSVG'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION AsSVG(geometry,int4)
	RETURNS TEXT
	AS 'MODULE_PATHNAME','LWGEOM_asSVG'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION AsSVG(geometry,int4,int4)
	RETURNS TEXT
	AS 'MODULE_PATHNAME','LWGEOM_asSVG'
	LANGUAGE 'C' IMMUTABLE STRICT;

-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION azimuth(geometry,geometry)
	RETURNS float8
	AS 'MODULE_PATHNAME', 'LWGEOM_azimuth'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION BdPolyFromText(text, integer)
RETURNS geometry
AS $$
DECLARE
	geomtext alias for $1;
	srid alias for $2;
	mline geometry;
	geom geometry;
BEGIN
	mline := ST_MultiLineStringFromText(geomtext, srid);

	IF mline IS NULL
	THEN
		RAISE EXCEPTION 'Input is not a MultiLinestring';
	END IF;

	geom := ST_BuildArea(mline);

	IF GeometryType(geom) != 'POLYGON'
	THEN
		RAISE EXCEPTION 'Input returns more then a single polygon, try using BdMPolyFromText instead';
	END IF;

	RETURN geom;
END;
$$
LANGUAGE 'plpgsql' IMMUTABLE STRICT;

-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION BdMPolyFromText(text, integer)
RETURNS geometry
AS $$
DECLARE
	geomtext alias for $1;
	srid alias for $2;
	mline geometry;
	geom geometry;
BEGIN
	mline := ST_MultiLineStringFromText(geomtext, srid);

	IF mline IS NULL
	THEN
		RAISE EXCEPTION 'Input is not a MultiLinestring';
	END IF;

	geom := ST_Multi(ST_BuildArea(mline));

	RETURN geom;
END;
$$
LANGUAGE 'plpgsql' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION boundary(geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME','boundary'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION buffer(geometry,float8,integer)
	RETURNS geometry
	AS 'SELECT ST_Buffer($1, $2, $3)'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION buffer(geometry,float8)
	RETURNS geometry
	AS 'MODULE_PATHNAME','buffer'
	LANGUAGE 'C' IMMUTABLE STRICT
	COST 100;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION BuildArea(geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_buildarea'
	LANGUAGE 'C' IMMUTABLE STRICT
	COST 100;
	
-- This is also available w/out GEOS
CREATE OR REPLACE FUNCTION Centroid(geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME'
	LANGUAGE 'C' IMMUTABLE STRICT;
		
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION Contains(geometry,geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION convexhull(geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME','convexhull'
	LANGUAGE 'C' IMMUTABLE STRICT
	COST 100;

-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION crosses(geometry,geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION distance(geometry,geometry)
	RETURNS float8
	AS 'MODULE_PATHNAME', 'LWGEOM_mindistance2d'
	LANGUAGE 'C' IMMUTABLE STRICT
	COST 100;
		
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION difference(geometry,geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME','difference'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION Dimension(geometry)
	RETURNS int4
	AS 'MODULE_PATHNAME', 'LWGEOM_dimension'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION disjoint(geometry,geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION distance_sphere(geometry,geometry)
	RETURNS FLOAT8
	AS 'MODULE_PATHNAME','LWGEOM_distance_sphere'
	LANGUAGE 'C' IMMUTABLE STRICT
	COST 100;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION distance_spheroid(geometry,geometry,spheroid)
	RETURNS FLOAT8
	AS 'MODULE_PATHNAME','LWGEOM_distance_ellipsoid'
	LANGUAGE 'C' IMMUTABLE STRICT
	COST 100;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION Dump(geometry)
	RETURNS SETOF geometry_dump
	AS 'MODULE_PATHNAME', 'LWGEOM_dump'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION DumpRings(geometry)
	RETURNS SETOF geometry_dump
	AS 'MODULE_PATHNAME', 'LWGEOM_dump_rings'
	LANGUAGE 'C' IMMUTABLE STRICT;

-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION Envelope(geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_envelope'
	LANGUAGE 'C' IMMUTABLE STRICT;
		
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION Expand(box2d,float8)
	RETURNS box2d
	AS 'MODULE_PATHNAME', 'BOX2DFLOAT4_expand'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION Expand(box3d,float8)
	RETURNS box3d
	AS 'MODULE_PATHNAME', 'BOX3D_expand'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION Expand(geometry,float8)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_expand'
	LANGUAGE 'C' IMMUTABLE STRICT;
		
CREATE AGGREGATE Extent(
	sfunc = ST_combine_bbox,
	basetype = geometry,
	finalfunc = box2d,
	stype = box3d
	);
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION Find_Extent(text,text) RETURNS box2d AS
$$
DECLARE
	tablename alias for $1;
	columnname alias for $2;
	myrec RECORD;

BEGIN
	FOR myrec IN EXECUTE 'SELECT ST_Extent("' || columnname || '") As extent FROM "' || tablename || '"' LOOP
		return myrec.extent;
	END LOOP;
END;
$$
LANGUAGE 'plpgsql' IMMUTABLE STRICT;

-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION Find_Extent(text,text,text) RETURNS box2d AS
$$
DECLARE
	schemaname alias for $1;
	tablename alias for $2;
	columnname alias for $3;
	myrec RECORD;

BEGIN
	FOR myrec IN EXECUTE 'SELECT ST_Extent("' || columnname || '") FROM "' || schemaname || '"."' || tablename || '" As extent ' LOOP
		return myrec.extent;
	END LOOP;
END;
$$
LANGUAGE 'plpgsql' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION EndPoint(geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_endpoint_linestring'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION ExteriorRing(geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME','LWGEOM_exteriorring_polygon'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION Force_2d(geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_force_2d'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- an alias for force_3dz
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION Force_3d(geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_force_3dz'
	LANGUAGE 'C' IMMUTABLE STRICT;
	

-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION Force_3dm(geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_force_3dm'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION Force_3dz(geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_force_3dz'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION Force_4d(geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_force_4d'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION Force_Collection(geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_force_collection'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION ForceRHR(geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_force_clockwise_poly'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION GeomCollFromText(text, int4)
	RETURNS geometry
	AS '
	SELECT CASE
	WHEN geometrytype(GeomFromText($1, $2)) = ''GEOMETRYCOLLECTION''
	THEN GeomFromText($1,$2)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION GeomCollFromText(text)
	RETURNS geometry
	AS '
	SELECT CASE
	WHEN geometrytype(GeomFromText($1)) = ''GEOMETRYCOLLECTION''
	THEN GeomFromText($1)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;

-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION GeomCollFromWKB(bytea, int)
	RETURNS geometry
	AS '
	SELECT CASE
	WHEN geometrytype(GeomFromWKB($1, $2)) = ''GEOMETRYCOLLECTION''
	THEN GeomFromWKB($1, $2)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;

	-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION GeomCollFromWKB(bytea)
	RETURNS geometry
	AS '
	SELECT CASE
	WHEN geometrytype(GeomFromWKB($1)) = ''GEOMETRYCOLLECTION''
	THEN GeomFromWKB($1)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION GeometryN(geometry,integer)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_geometryn_collection'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION GeomUnion(geometry,geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME','geomunion'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Availability: 1.5.0  -- replaced with postgis_getbbox
CREATE OR REPLACE FUNCTION getbbox(geometry)
	RETURNS box2d
	AS 'MODULE_PATHNAME','LWGEOM_to_BOX2DFLOAT4'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION intersects(geometry,geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION IsRing(geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION IsSimple(geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'issimple'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION length_spheroid(geometry, spheroid)
	RETURNS FLOAT8
	AS 'MODULE_PATHNAME','LWGEOM_length_ellipsoid_linestring'
	LANGUAGE 'C' IMMUTABLE STRICT
	COST 100;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION length2d_spheroid(geometry, spheroid)
	RETURNS FLOAT8
	AS 'MODULE_PATHNAME','LWGEOM_length2d_ellipsoid'
	LANGUAGE 'C' IMMUTABLE STRICT
	COST 100;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION length3d_spheroid(geometry, spheroid)
	RETURNS FLOAT8
	AS 'MODULE_PATHNAME','LWGEOM_length_ellipsoid_linestring'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION LineMerge(geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'linemerge'
	LANGUAGE 'C' IMMUTABLE STRICT
	COST 100;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION locate_along_measure(geometry, float8)
	RETURNS geometry
	AS $$ SELECT ST_locate_between_measures($1, $2, $2) $$
	LANGUAGE 'sql' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MakeBox2d(geometry, geometry)
	RETURNS box2d
	AS 'MODULE_PATHNAME', 'BOX2DFLOAT4_construct'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MakePolygon(geometry, geometry[])
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_makepoly'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MakePolygon(geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_makepoly'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MPolyFromWKB(bytea)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(GeomFromWKB($1)) = ''MULTIPOLYGON''
	THEN GeomFromWKB($1)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
		
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION multi(geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_force_multi'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MultiPolyFromWKB(bytea, int)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(GeomFromWKB($1, $2)) = ''MULTIPOLYGON''
	THEN GeomFromWKB($1, $2)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MultiPolyFromWKB(bytea)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(GeomFromWKB($1)) = ''MULTIPOLYGON''
	THEN GeomFromWKB($1)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION InteriorRingN(geometry,integer)
	RETURNS geometry
	AS 'MODULE_PATHNAME','LWGEOM_interiorringn_polygon'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION intersection(geometry,geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME','intersection'
	LANGUAGE 'C' IMMUTABLE STRICT;

	-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION IsClosed(geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'LWGEOM_isclosed'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION IsEmpty(geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'LWGEOM_isempty'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION IsValid(geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'isvalid'
	LANGUAGE 'C' IMMUTABLE STRICT
	COST 100;
-- this is a fake (for back-compatibility)
-- uses 3d if 3d is available, 2d otherwise
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION length3d(geometry)
	RETURNS FLOAT8
	AS 'MODULE_PATHNAME', 'LWGEOM_length_linestring'
	LANGUAGE 'C' IMMUTABLE STRICT;

-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION length2d(geometry)
	RETURNS FLOAT8
	AS 'MODULE_PATHNAME', 'LWGEOM_length2d_linestring'
	LANGUAGE 'C' IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION length(geometry)
	RETURNS FLOAT8
	AS 'MODULE_PATHNAME', 'LWGEOM_length_linestring'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION line_interpolate_point(geometry, float8)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_line_interpolate_point'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION line_locate_point(geometry, geometry)
	RETURNS float8
	AS 'MODULE_PATHNAME', 'LWGEOM_line_locate_point'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION line_substring(geometry, float8, float8)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_line_substring'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION LineFromText(text)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(GeomFromText($1)) = ''LINESTRING''
	THEN GeomFromText($1)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION LineFromText(text, int4)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(GeomFromText($1, $2)) = ''LINESTRING''
	THEN GeomFromText($1,$2)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION LineFromMultiPoint(geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_line_from_mpoint'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION LineFromWKB(bytea, int)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(GeomFromWKB($1, $2)) = ''LINESTRING''
	THEN GeomFromWKB($1, $2)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION LineFromWKB(bytea)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(GeomFromWKB($1)) = ''LINESTRING''
	THEN GeomFromWKB($1)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION LineStringFromText(text)
	RETURNS geometry
	AS 'SELECT LineFromText($1)'
	LANGUAGE 'SQL' IMMUTABLE STRICT;

-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION LineStringFromText(text, int4)
	RETURNS geometry
	AS 'SELECT LineFromText($1, $2)'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION LinestringFromWKB(bytea, int)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(GeomFromWKB($1, $2)) = ''LINESTRING''
	THEN GeomFromWKB($1, $2)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION LinestringFromWKB(bytea)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(GeomFromWKB($1)) = ''LINESTRING''
	THEN GeomFromWKB($1)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;

-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION locate_between_measures(geometry, float8, float8)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_locate_between_m'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION M(geometry)
	RETURNS float8
	AS 'MODULE_PATHNAME','LWGEOM_m_point'
	LANGUAGE 'C' IMMUTABLE STRICT;

-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MakeBox3d(geometry, geometry)
	RETURNS box3d
	AS 'MODULE_PATHNAME', 'BOX3D_construct'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION makeline_garray (geometry[])
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_makeline_garray'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE AGGREGATE makeline (
	BASETYPE = geometry,
	SFUNC = pgis_geometry_accum_transfn,
	STYPE = pgis_abs,
	FINALFUNC = pgis_geometry_makeline_finalfn
	);
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MakeLine(geometry, geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_makeline'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MakePoint(float8, float8)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_makepoint'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MakePoint(float8, float8, float8)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_makepoint'
	LANGUAGE 'C' IMMUTABLE STRICT;

	-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MakePoint(float8, float8, float8, float8)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_makepoint'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MakePointM(float8, float8, float8)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_makepoint3dm'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- This should really be deprecated -- 2011-01-04 robe
CREATE OR REPLACE FUNCTION max_distance(geometry,geometry)
	RETURNS float8
	AS 'MODULE_PATHNAME', 'LWGEOM_maxdistance2d_linestring'
	LANGUAGE 'C' IMMUTABLE STRICT; 
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION mem_size(geometry)
	RETURNS int4
	AS 'MODULE_PATHNAME', 'LWGEOM_mem_size'
	LANGUAGE 'C' IMMUTABLE STRICT;

-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MLineFromText(text, int4)
	RETURNS geometry
	AS '
	SELECT CASE
	WHEN geometrytype(GeomFromText($1, $2)) = ''MULTILINESTRING''
	THEN GeomFromText($1,$2)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MLineFromText(text)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(GeomFromText($1)) = ''MULTILINESTRING''
	THEN GeomFromText($1)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MLineFromWKB(bytea, int)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(GeomFromWKB($1, $2)) = ''MULTILINESTRING''
	THEN GeomFromWKB($1, $2)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MLineFromWKB(bytea)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(GeomFromWKB($1)) = ''MULTILINESTRING''
	THEN GeomFromWKB($1)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;

-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MPointFromText(text, int4)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(GeomFromText($1,$2)) = ''MULTIPOINT''
	THEN GeomFromText($1,$2)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;

-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MPointFromText(text)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(GeomFromText($1)) = ''MULTIPOINT''
	THEN GeomFromText($1)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;

-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MPointFromWKB(bytea, int)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(GeomFromWKB($1,$2)) = ''MULTIPOINT''
	THEN GeomFromWKB($1, $2)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;

-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MPointFromWKB(bytea)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(GeomFromWKB($1)) = ''MULTIPOINT''
	THEN GeomFromWKB($1)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MPolyFromText(text, int4)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(GeomFromText($1, $2)) = ''MULTIPOLYGON''
	THEN GeomFromText($1,$2)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;

-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MPolyFromText(text)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(GeomFromText($1)) = ''MULTIPOLYGON''
	THEN GeomFromText($1)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MPolyFromWKB(bytea, int)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(GeomFromWKB($1, $2)) = ''MULTIPOLYGON''
	THEN GeomFromWKB($1, $2)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MultiLineFromWKB(bytea, int)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(GeomFromWKB($1, $2)) = ''MULTILINESTRING''
	THEN GeomFromWKB($1, $2)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;

-- Availability: 1.2.2
CREATE OR REPLACE FUNCTION MultiLineFromWKB(bytea, int)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(GeomFromWKB($1, $2)) = ''MULTILINESTRING''
	THEN GeomFromWKB($1, $2)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;

-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MultiLineFromWKB(bytea)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(GeomFromWKB($1)) = ''MULTILINESTRING''
	THEN GeomFromWKB($1)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MultiLineStringFromText(text)
	RETURNS geometry
	AS 'SELECT ST_MLineFromText($1)'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MultiLineStringFromText(text, int4)
	RETURNS geometry
	AS 'SELECT MLineFromText($1, $2)'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MultiPointFromText(text)
	RETURNS geometry
	AS 'SELECT MPointFromText($1)'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MultiPointFromText(text)
	RETURNS geometry
	AS 'SELECT MPointFromText($1)'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MultiPointFromText(text, int4)
	RETURNS geometry
	AS 'SELECT MPointFromText($1, $2)'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MultiPointFromWKB(bytea, int)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(GeomFromWKB($1,$2)) = ''MULTIPOINT''
	THEN GeomFromWKB($1, $2)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MultiPointFromWKB(bytea)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(GeomFromWKB($1)) = ''MULTIPOINT''
	THEN GeomFromWKB($1)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MultiPolygonFromText(text, int4)
	RETURNS geometry
	AS 'SELECT MPolyFromText($1, $2)'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION MultiPolygonFromText(text)
	RETURNS geometry
	AS 'SELECT MPolyFromText($1)'
	LANGUAGE 'SQL' IMMUTABLE STRICT;

-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION NumInteriorRing(geometry)
	RETURNS integer
	AS 'MODULE_PATHNAME','LWGEOM_numinteriorrings_polygon'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION NumInteriorRings(geometry)
	RETURNS integer
	AS 'MODULE_PATHNAME','LWGEOM_numinteriorrings_polygon'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION npoints(geometry)
	RETURNS int4
	AS 'MODULE_PATHNAME', 'LWGEOM_npoints'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION nrings(geometry)
	RETURNS int4
	AS 'MODULE_PATHNAME', 'LWGEOM_nrings'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION NumGeometries(geometry)
	RETURNS int4
	AS 'MODULE_PATHNAME', 'LWGEOM_numgeometries_collection'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION NumPoints(geometry)
	RETURNS int4
	AS 'MODULE_PATHNAME', 'LWGEOM_numpoints_linestring'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION overlaps(geometry,geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- this is a fake (for back-compatibility)
-- uses 3d if 3d is available, 2d otherwise
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION perimeter3d(geometry)
	RETURNS FLOAT8
	AS 'MODULE_PATHNAME', 'LWGEOM_perimeter_poly'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION perimeter2d(geometry)
	RETURNS FLOAT8
	AS 'MODULE_PATHNAME', 'LWGEOM_perimeter2d_poly'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION point_inside_circle(geometry,float8,float8,float8)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'LWGEOM_inside_circle_point'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION PointFromText(text)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(GeomFromText($1)) = ''POINT''
	THEN GeomFromText($1)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;

-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION PointFromText(text, int4)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(GeomFromText($1, $2)) = ''POINT''
	THEN GeomFromText($1,$2)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION PointFromWKB(bytea)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(GeomFromWKB($1)) = ''POINT''
	THEN GeomFromWKB($1)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION PointFromWKB(bytea, int)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(ST_GeomFromWKB($1, $2)) = ''POINT''
	THEN GeomFromWKB($1, $2)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;

	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION PointN(geometry,integer)
	RETURNS geometry
	AS 'MODULE_PATHNAME','LWGEOM_pointn_linestring'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION PointOnSurface(geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME'
	LANGUAGE 'C' IMMUTABLE STRICT;

	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION PolyFromText(text)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(GeomFromText($1)) = ''POLYGON''
	THEN GeomFromText($1)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION PolyFromText(text, int4)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(GeomFromText($1, $2)) = ''POLYGON''
	THEN GeomFromText($1,$2)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION PolyFromWKB(bytea, int)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(GeomFromWKB($1, $2)) = ''POLYGON''
	THEN GeomFromWKB($1, $2)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION PolyFromWKB(bytea)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(GeomFromWKB($1)) = ''POLYGON''
	THEN GeomFromWKB($1)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION PolygonFromText(text, int4)
	RETURNS geometry
	AS 'SELECT PolyFromText($1, $2)'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION PolygonFromText(text)
	RETURNS geometry
	AS 'SELECT PolyFromText($1)'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION PolygonFromWKB(bytea, int)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(GeomFromWKB($1,$2)) = ''POLYGON''
	THEN GeomFromWKB($1, $2)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;

	-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION PolygonFromWKB(bytea)
	RETURNS geometry
	AS '
	SELECT CASE WHEN geometrytype(GeomFromWKB($1)) = ''POLYGON''
	THEN GeomFromWKB($1)
	ELSE NULL END
	'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION Polygonize_GArray (geometry[])
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'polygonize_garray'
	LANGUAGE 'C' IMMUTABLE STRICT
	COST 100;

-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION relate(geometry,geometry)
	RETURNS text
	AS 'MODULE_PATHNAME','relate_full'
	LANGUAGE 'C' IMMUTABLE STRICT;

-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION relate(geometry,geometry,text)
	RETURNS boolean
	AS 'MODULE_PATHNAME','relate_pattern'
	LANGUAGE 'C' IMMUTABLE STRICT;	

-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION RemovePoint(geometry, integer)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_removepoint'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION reverse(geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_reverse'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION Segmentize(geometry, float8)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_segmentize2d'
	LANGUAGE 'C' IMMUTABLE STRICT;	
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION SetPoint(geometry, integer, geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_setpoint_linestring'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Availability: 1.1.0
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION shift_longitude(geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_longitude_shift'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION Simplify(geometry, float8)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_simplify2d'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- SnapToGrid(input, size) # xsize=ysize=size, offsets=0
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION SnapToGrid(geometry, float8, float8, float8, float8)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_snaptogrid'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION SnapToGrid(geometry, float8)
	RETURNS geometry
	AS 'SELECT ST_SnapToGrid($1, 0, 0, $2, $2)'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- SnapToGrid(input, point_offsets, xsize, ysize, zsize, msize)
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION SnapToGrid(geometry, geometry, float8, float8, float8, float8)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_snaptogrid_pointoff'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- SnapToGrid(input, xsize, ysize) # offsets=0
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION SnapToGrid(geometry, float8, float8)
	RETURNS geometry
	AS 'SELECT ST_SnapToGrid($1, 0, 0, $2, $3)'
	LANGUAGE 'SQL' IMMUTABLE STRICT;
	
-- Availability: 1.2.2 -- this should be deprecated (do not think anyone has ever used it)
CREATE OR REPLACE FUNCTION ST_MakeLine_GArray (geometry[])
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_makeline_garray'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION StartPoint(geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_startpoint_linestring'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION symdifference(geometry,geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME','symdifference'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION symmetricdifference(geometry,geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME','symdifference'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION summary(geometry)
	RETURNS text
	AS 'MODULE_PATHNAME', 'LWGEOM_summary'
	LANGUAGE 'C' IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION transform(geometry,integer)
	RETURNS geometry
	AS 'MODULE_PATHNAME','transform'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION touches(geometry,geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME'
	LANGUAGE 'C' IMMUTABLE STRICT;

-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION within(geometry,geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION X(geometry)
	RETURNS float8
	AS 'MODULE_PATHNAME','LWGEOM_x_point'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION xmax(box3d)
	RETURNS FLOAT8
	AS 'MODULE_PATHNAME','BOX3D_xmax'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION xmin(box3d)
	RETURNS FLOAT8
	AS 'MODULE_PATHNAME','BOX3D_xmin'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION Y(geometry)
	RETURNS float8
	AS 'MODULE_PATHNAME','LWGEOM_y_point'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION ymax(box3d)
	RETURNS FLOAT8
	AS 'MODULE_PATHNAME','BOX3D_ymax'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION ymin(box3d)
	RETURNS FLOAT8
	AS 'MODULE_PATHNAME','BOX3D_ymin'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION Z(geometry)
	RETURNS float8
	AS 'MODULE_PATHNAME','LWGEOM_z_point'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION zmax(box3d)
	RETURNS FLOAT8
	AS 'MODULE_PATHNAME','BOX3D_zmax'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION zmin(box3d)
	RETURNS FLOAT8
	AS 'MODULE_PATHNAME','BOX3D_zmin'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION zmflag(geometry)
	RETURNS smallint
	AS 'MODULE_PATHNAME', 'LWGEOM_zmflag'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- end old ogc names that have been replaced with new SQL-MM names --

--- Start Aggregates and supporting functions --
-- Deprecation in: 1.2.3
CREATE AGGREGATE accum (
	sfunc = pgis_geometry_accum_transfn,
	basetype = geometry,
	stype = pgis_abs,
	finalfunc = pgis_geometry_accum_finalfn
	);
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION collect(geometry, geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'LWGEOM_collect'
	LANGUAGE 'C' IMMUTABLE;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION combine_bbox(box2d,geometry)
	RETURNS box2d
	AS 'MODULE_PATHNAME', 'BOX2DFLOAT4_combine'
	LANGUAGE 'C' IMMUTABLE;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION combine_bbox(box3d,geometry)
	RETURNS box3d
	AS 'MODULE_PATHNAME', 'BOX3D_combine'
	LANGUAGE 'C' IMMUTABLE;

	-- Deprecation in 1.5.0
CREATE OR REPLACE FUNCTION ST_Polygonize_GArray (geometry[])
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'polygonize_garray'
	LANGUAGE 'C' IMMUTABLE STRICT
	COST 100;

-- Deprecation in 1.4.0
CREATE OR REPLACE FUNCTION ST_unite_garray (geometry[])
	RETURNS geometry
	AS 'MODULE_PATHNAME','pgis_union_geometry_array'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE OR REPLACE FUNCTION unite_garray (geometry[])
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'pgis_union_geometry_array'
	LANGUAGE 'C' IMMUTABLE STRICT;
	
-- Deprecation in 1.2.3
CREATE AGGREGATE Extent3d(
	sfunc = combine_bbox,
	basetype = geometry,
	stype = box3d
	);
	
-- Deprecation in 1.2.3
CREATE AGGREGATE memcollect(
	sfunc = ST_collect,
	basetype = geometry,
	stype = geometry
	);

-- Deprecation in 1.2.3
CREATE AGGREGATE MemGeomUnion (
	basetype = geometry,
	sfunc = geomunion,
	stype = geometry
	);

-- End Aggregates and supporting functions --
------------------------------------------------
--Begin 3D functions --
------------------------------------------------

-- Renamed in 2.0.0 to ST_3DLength
CREATE OR REPLACE FUNCTION ST_Length3D(geometry)
	RETURNS FLOAT8
	AS 'MODULE_PATHNAME', 'LWGEOM_length_linestring'
	LANGUAGE 'C' IMMUTABLE STRICT;

-- Renamed in 2.0.0 to ST_3DLength_spheroid
CREATE OR REPLACE FUNCTION ST_Length_spheroid3D(geometry, spheroid)
	RETURNS FLOAT8
	AS 'MODULE_PATHNAME','LWGEOM_length_ellipsoid_linestring'
	LANGUAGE 'C' IMMUTABLE STRICT
	COST 100;
	
-- Renamed in 2.0.0 to ST_3DPerimeter
CREATE OR REPLACE FUNCTION ST_Perimeter3D(geometry)
	RETURNS FLOAT8
	AS 'MODULE_PATHNAME', 'LWGEOM_perimeter_poly'
	LANGUAGE 'C' IMMUTABLE STRICT;

-- Renamed in 2.0.0 to ST_3DMakeBox
CREATE OR REPLACE FUNCTION ST_MakeBox3D(geometry, geometry)
	RETURNS box3d
	AS 'MODULE_PATHNAME', 'BOX3D_construct'
	LANGUAGE 'C' IMMUTABLE STRICT;

-- Renamed in 2.0.0 to ST_3DExtent
CREATE AGGREGATE ST_Extent3D(
	sfunc = ST_combine_bbox,
	basetype = geometry,
	stype = box3d
	);
--END 3D functions--
