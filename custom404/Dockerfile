FROM nginx:alpine

# Copy the HTML file
COPY 404.html /usr/share/nginx/html/404.html

# Create the images directory and copy the images
RUN mkdir -p /usr/share/nginx/html/images
COPY dead.png /usr/share/nginx/html/images/dead.png
COPY time.png /usr/share/nginx/html/images/time.png
COPY unknown.png /usr/share/nginx/html/images/unknown.png

# Copy the nginx configuration
COPY nginx.conf /etc/nginx/nginx.conf

# Ensure correct file permissions
RUN chmod -R 755 /usr/share/nginx/html && \
    chmod 644 /etc/nginx/nginx.conf

# Validate nginx configuration
RUN nginx -t