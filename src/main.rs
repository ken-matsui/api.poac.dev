mod auth;
mod constants;
mod health;
mod package;
mod routes;
mod schema;
mod user;

use std::env;

use actix_web::{middleware::Logger, web, web::ServiceConfig};
use diesel::r2d2::{ConnectionManager, Pool};
use diesel::PgConnection;
use shuttle_actix_web::ShuttleActixWeb;
use shuttle_secrets::SecretStore;

#[shuttle_runtime::main]
async fn actix_web(
    #[shuttle_secrets::Secrets] secret_store: SecretStore,
) -> ShuttleActixWeb<impl FnOnce(&mut ServiceConfig) + Send + Clone + 'static> {
    // Set up secrets
    let database_url = secret_store.get("DATABASE_URL").expect("DATABASE_URL must be set");
    let gh_oauth_client_id = secret_store.get("GITHUB_OAUTH_CLIENT_ID").expect("GITHUB_OAUTH_CLIENT_ID must be set");
    let gh_oauth_client_secret = secret_store.get("GITHUB_OAUTH_CLIENT_SECRET").expect("GITHUB_OAUTH_CLIENT_SECRET must be set");

    env::set_var("GITHUB_OAUTH_CLIENT_ID", gh_oauth_client_id);
    env::set_var("GITHUB_OAUTH_CLIENT_SECRET", gh_oauth_client_secret);

    // Set up database connection pool
    let manager = ConnectionManager::<PgConnection>::new(database_url);
    let pool = Pool::builder()
        .build(manager)
        .expect("Failed to create pool.");
    // Set up DB pool to be used with web::Data<Pool> extractor
    let state = web::Data::new(pool.clone());

    let host = env::var("HOST").unwrap_or("127.0.0.1".to_string());
    let port = env::var("PORT")
        .map(|port| port.parse::<u16>().expect("Failed to parse PORT as u16"))
        .unwrap_or(8000);
    log::info!("starting HTTP server at http://{host}:{port}");

    let config = move |cfg: &mut ServiceConfig| {
        cfg.app_data(state)
            .route("/health", web::get().to(health::api))
            .route("/health_db", web::get().to(health::db))
            // .wrap(Logger::default())  TODO: add logger
            .configure(routes::init_routes);
    };
    Ok(config.into())
}
