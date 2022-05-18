-- Create tables

create table public.users (
    id uuid primary key references auth.users,
    name text not null,
    user_name text not null,
    avatar_url text not null
);

-- Create RLS policies

alter table public.users
    enable row level security;

create policy "Allow all users to SELECT themselves"
    on public.users
    for select using (
        true
    );

create policy "Allow users to UPDATE themselves"
	on public.users
	for update using (
        auth.uid() = id
    );

-- Create functions

create function public.handle_new_user()
returns trigger
language plpgsql
security definer set search_path = public -- to bypass RLS
as $$
begin
    insert into public.users (id, name, user_name, avatar_url)
    values (
        new.id,
        new.raw_user_meta_data->>'name',
        new.raw_user_meta_data->>'user_name',
        new.raw_user_meta_data->>'avatar_url'
    );
    return new;
end;
$$;

create function public.handle_updated_user()
returns trigger
language plpgsql
security definer set search_path = public -- to bypass RLS
as $$
begin
    update public.users
    set (name, user_name, avatar_url) = (
        new.raw_user_meta_data->>'name',
        new.raw_user_meta_data->>'user_name',
        new.raw_user_meta_data->>'avatar_url'
    );
    return new;
end;
$$;

-- Create triggers

create trigger on_auth_user_created
    after insert on auth.users
    for each row execute procedure public.handle_new_user();

create trigger on_auth_user_updated
    after update on auth.users
    for each row execute procedure public.handle_updated_user();

-- Create functions

create function get_owners(pkgname text)
returns table (
    id uuid,
    name text,
    user_name text,
    avatar_url text
)
language sql
as $$
    select u.id, name, user_name, avatar_url
    from public.users u
    left join ownerships o on o.package_name = pkgname
    where o.user_id = u.id;
$$;
