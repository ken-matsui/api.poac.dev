// @generated automatically by Diesel CLI.

diesel::table! {
    packages (id) {
        id -> Uuid,
        published_at -> Timestamptz,
        metadata -> Jsonb,
        name -> Text,
        version -> Text,
        edition -> Int2,
        authors -> Array<Text>,
        repository -> Text,
        description -> Text,
        license -> Text,
        readme -> Nullable<Text>,
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

diesel::allow_tables_to_appear_in_same_query!(packages, users);
