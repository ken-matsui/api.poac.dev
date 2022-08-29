create table public.packages (
    id uuid primary key default uuid_generate_v4(),
    published_at timestamptz not null default now(),
    metadata jsonb not null unique,
    name text not null,
    version text not null,
    edition int2 not null,
    authors text[] not null,
    repository text not null,
    description text not null,
    license text not null,
    readme text, -- has markdown
    sha256sum varchar(64) not null,

    unique (name, version)
);
