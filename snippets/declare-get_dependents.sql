drop function get_dependents;

create or replace function get_dependents(depname text)
returns table (
  id uuid,
  published_at timestamp,
  metadata jsonb,
  name text,
  version text,
  edition int2,
  authors text[],
  repository text,
  description text,
  license text,
  readme text,
  sha256sum varchar(64)
)
language sql
as $$
  select * from packages
  where metadata->'dependencies' ? depname = true;
$$;

