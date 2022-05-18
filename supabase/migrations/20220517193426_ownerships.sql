-- Create tables

create table public.ownerships (
    id uuid primary key default uuid_generate_v4(),
    package_name text not null,
    user_id uuid not null references auth.users(id) default auth.uid(),
    is_pending boolean not null default false,

    unique (package_name, user_id)
);

-- Create RLS policies

alter table public.ownerships
    enable row level security;

create policy "Allow existing owners to INSERT"
    on public.ownerships
    for insert with check (
        auth.uid() in (
            select user_id from ownerships o
            where o.package_name = package_name
              and not is_pending
        )
    );

create policy "Allow all users to SELECT ownerships"
    on public.ownerships
    for select using (
        true
    );

create policy "Allow owners to UPDATE themselves"
    on public.ownerships
    for update using (
        auth.uid() = user_id
    );

create policy "The last owner should be kept by DELETING"
    on public.ownerships
    for delete using (
        (auth.uid() in (
            select user_id from ownerships o
            where o.package_name = package_name
              and not is_pending
        )) and
        (select count(*) from ownerships o
            where o.package_name = package_name
              and not is_pending
        ) > 1
    );

-- Create functions
