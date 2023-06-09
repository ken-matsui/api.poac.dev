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

