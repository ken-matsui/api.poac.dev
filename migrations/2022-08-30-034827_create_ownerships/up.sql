create table public.ownerships (
    id uuid primary key default uuid_generate_v4(),
    package_name text not null,
    user_id uuid not null references public.users(id),
    is_pending boolean not null default false,

    unique (package_name, user_id)
);
