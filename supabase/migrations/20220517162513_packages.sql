-- Create tables

create table public.packages (
    id uuid primary key default uuid_generate_v4(),
    published_at timestampz not null default now(),
    metadata jsonb not null unique,
    name text not null, -- TODO: varchar(?)
    version text not null, -- TODO: varchar(?)
    edition int2 not null, -- TODO: varchar(?)
    authors text[] not null, -- TODO: varchar(?)
    repository text not null, -- TODO: varchar(?)
    description text not null, -- TODO: varchar(?)
    license text not null, -- TODO: varchar(?)
    readme text, -- has markdown
    sha256sum varchar(64) not null,

    unique (name, version)
);

-- Create RLS policies

alter table public.packages
    enable row level security;

create policy "Allow all users to SELECT"
    on public.packages
    for select using (
        true
    );

-- Create functions

-- Get packages ordered by version (newer first)
create function get_packages()
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
    order by name, string_to_array(version, '.')::int[] desc;
$$;

-- Get packages with latest version
create function get_uniq_packages()
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
    select distinct on (name) * from packages
    order by name, string_to_array(version, '.')::int[] desc;
$$;

-- Get dependents of given package name
create function get_dependents(depname text)
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
