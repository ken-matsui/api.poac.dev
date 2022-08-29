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
