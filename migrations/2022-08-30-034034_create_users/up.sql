create table public.users (
    id uuid primary key default uuid_generate_v4(),
    user_name text not null unique,
    email text not null,
    name text not null,
    avatar_url text not null,
    status text not null -- active, disabled - reason
);
