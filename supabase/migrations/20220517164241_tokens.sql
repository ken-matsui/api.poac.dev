-- Create tables

create table public.tokens (
    id uuid primary key default uuid_generate_v4(),
    created_at timestampz not null default now(),
    last_used_at timestampz,
    token text not null unique, -- TODO: varchar(?)
    owner_id uuid not null references auth.users(id) default auth.uid(),
    name text not null, -- TODO: varchar(?)
);

-- Create RLS policies

alter table public.tokens
    enable row level security;

create policy "Allow INSERT up to 5 tokens"
    on public.tokens
    for insert check (
        (select count(*)
            from tokens
            where (auth.uid() = owner_id)) <= 5
    );

create policy "Allow SELECT for owners"
    on public.tokens
    for select using (
        auth.uid() = owner_id
    );

create policy "Allow DELETE for owners"
    on public.tokens
    for delete using (
        auth.uid() = owner_id
    );

-- Create functions

-- Create a new token
-- \return {f1: uuid, f2: timestamp}
create function create_token(name text, token text)
language plpgsql
returns record
as $$
declare
    id_v uuid = uuid_generate_v4();
    created_at_v timestamp := now();
begin
    insert into public.tokens (id, created_at, name, token)
    values (id_v, created_at_v, name, crypt(token, gen_salt('bf')));
    return (id_v, created_at_v);
end
$$;
