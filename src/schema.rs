// @generated automatically by Diesel CLI.

diesel::table! {
    officials (official_name) {
        official_name -> Text,
        original_name -> Text,
    }
}

diesel::table! {
    ownerships (id) {
        id -> Uuid,
        package_name -> Text,
        user_id -> Uuid,
        is_pending -> Bool,
    }
}

diesel::table! {
    packages (id) {
        id -> Uuid,
        published_at -> Timestamptz,
        metadata -> Jsonb,
        name -> Text,
        version -> Text,
        edition -> Int2,
        authors -> Array<Nullable<Text>>,
        repository -> Text,
        description -> Text,
        license -> Text,
        readme -> Nullable<Text>,
        #[max_length = 64]
        sha256sum -> Varchar,
    }
}

diesel::table! {
    users (id) {
        id -> Uuid,
        user_name -> Text,
        email -> Text,
        name -> Text,
        avatar_url -> Text,
        status -> Text,
    }
}

diesel::joinable!(ownerships -> users (user_id));

diesel::allow_tables_to_appear_in_same_query!(
    officials,
    ownerships,
    packages,
    users,
);
