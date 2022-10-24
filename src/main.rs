mod auth;
mod package;
mod routes;
mod schema;
mod user;

use actix_web::{middleware::Logger, web, App, HttpResponse, HttpServer, Result};
use diesel::dsl::sql;
use diesel::r2d2::{ConnectionManager, Pool};
use diesel::sql_types::Bool;
use diesel::{PgConnection, RunQueryDsl};
use dotenvy::dotenv;
use poac_api_utils::DbPool;
use std::env;

async fn get_health_status(pool: web::Data<DbPool>) -> Result<HttpResponse> {
    let is_database_connected: bool = web::block(move || {
        if let Ok(mut conn) = pool.get() {
            sql::<Bool>("SELECT 1").execute(&mut conn).is_ok()
        } else {
            false
        }
    })
    .await?;

    let body = serde_json::json!({ "database_connected": is_database_connected }).to_string();
    if is_database_connected {
        Ok(HttpResponse::Ok()
            .content_type("application/json")
            .body(body))
    } else {
        Ok(HttpResponse::ServiceUnavailable()
            .content_type("application/json")
            .body(body))
    }
}

#[actix_web::main]
async fn main() -> std::io::Result<()> {
    dotenv().ok();
    env_logger::init_from_env(env_logger::Env::new().default_filter_or("info"));

    // Set up database connection pool
    let database_url = env::var("DATABASE_URL").expect("DATABASE_URL must be set");
    let manager = ConnectionManager::<PgConnection>::new(database_url);
    let pool = Pool::builder()
        .build(manager)
        .expect("Failed to create pool.");

    let port = env::var("PORT")
        .map(|port| port.parse::<u16>().expect("Failed to parse PORT as u16"))
        .unwrap_or(8000);
    log::info!("starting HTTP server at http://localhost:{}", port);

    // Start HTTP server
    HttpServer::new(move || {
        App::new()
            // Set up DB pool to be used with web::Data<Pool> extractor
            .app_data(web::Data::new(pool.clone()))
            .route("/health", web::get().to(get_health_status))
            .wrap(Logger::default())
            .configure(routes::init_routes)
    })
    .bind(("127.0.0.1", port))?
    .run()
    .await
}
